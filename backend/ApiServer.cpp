#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "ChessBoard.hpp"
#include "GameState.hpp"
#include "third_party/nlohmann/json.hpp"

namespace
{
using json = nlohmann::json;

constexpr int PORT = 8787;
constexpr DWORD CLIENT_TIMEOUT_MS = 1000;
constexpr size_t MAX_REQUEST_SIZE = 64 * 1024;

std::string trim(const std::string &value)
{
    const auto start = value.find_first_not_of(" \r\n\t");
    if (start == std::string::npos)
    {
        return "";
    }

    const auto end = value.find_last_not_of(" \r\n\t");
    return value.substr(start, end - start + 1);
}

std::string frontendSquareFor(int row, int col)
{
    std::string square;
    square += static_cast<char>('a' + col);
    square += static_cast<char>('8' - row);
    return square;
}

char symbolForPiece(Pieces *piece)
{
    if (piece == nullptr)
    {
        return '\0';
    }

    char symbol = '\0';
    const std::string type = piece->getType();

    if (type == "Pawn")
    {
        symbol = 'p';
    }
    else if (type == "Knight")
    {
        symbol = 'n';
    }
    else if (type == "Bishop")
    {
        symbol = 'b';
    }
    else if (type == "Rook")
    {
        symbol = 'r';
    }
    else if (type == "Queen")
    {
        symbol = 'q';
    }
    else if (type == "King")
    {
        symbol = 'k';
    }

    if (piece->getColor() == "White")
    {
        symbol = static_cast<char>(std::toupper(static_cast<unsigned char>(symbol)));
    }

    return symbol;
}

json boardToJson(Board &board)
{
    json frontendBoard = json::array();

    for (int row = 0; row < 8; row++)
    {
        json frontendRow = json::array();
        for (int col = 0; col < 8; col++)
        {
            Pieces *piece = board.getPieceAt(frontendSquareFor(row, col));
            char symbol = symbolForPiece(piece);
            frontendRow.push_back(symbol == '\0' ? "" : std::string(1, symbol));
        }
        frontendBoard.push_back(frontendRow);
    }

    return frontendBoard;
}

std::string cardTimingToString(CardTiming timing)
{
    switch (timing)
    {
    case CardTiming::BEFORE_OWN_TURN:
        return "BEFORE_OWN_TURN";
    case CardTiming::AS_OWN_TURN:
        return "AS_OWN_TURN";
    case CardTiming::AFTER_OWN_TURN:
        return "AFTER_OWN_TURN";
    case CardTiming::AFTER_OPPONENT_TURN:
        return "AFTER_OPPONENT_TURN";
    case CardTiming::ANYTIME:
        return "ANYTIME";
    }
    return "";
}

std::string cardTargetToString(CardTargetRequirement target)
{
    switch (target)
    {
    case CardTargetRequirement::NONE:
        return "NONE";
    case CardTargetRequirement::FRIENDLY_NON_KING:
        return "FRIENDLY_NON_KING";
    case CardTargetRequirement::FRIENDLY_KNIGHT:
        return "FRIENDLY_KNIGHT";
    case CardTargetRequirement::FRIENDLY_PAWNS:
        return "FRIENDLY_PAWNS";
    case CardTargetRequirement::ENEMY_NON_KING_QUEEN:
        return "ENEMY_NON_KING_QUEEN";
    case CardTargetRequirement::FRIENDLY_KING:
        return "FRIENDLY_KING";
    }
    return "";
}

std::string cardEffectToString(CardEffectType effect)
{
    switch (effect)
    {
    case CardEffectType::NONE:
        return "NONE";
    case CardEffectType::DESTROY_TARGET:
        return "DESTROY_TARGET";
    case CardEffectType::CHARGE_KNIGHT:
        return "CHARGE_KNIGHT";
    case CardEffectType::MOVE_PAWNS:
        return "MOVE_PAWNS";
    case CardEffectType::CANCEL_LAST_ACTION:
        return "CANCEL_LAST_ACTION";
    case CardEffectType::BOG_MOVE:
        return "BOG_MOVE";
    case CardEffectType::APPLY_NEUTRALITY:
        return "APPLY_NEUTRALITY";
    case CardEffectType::APPLY_WARLORD:
        return "APPLY_WARLORD";
    }
    return "";
}

json cardDefinitionToJson(const CardDefinition &definition)
{
    return {
        {"id", definition.id},
        {"name", definition.name},
        {"level", definition.level},
        {"timing", cardTimingToString(definition.timing)},
        {"targetRequirement", cardTargetToString(definition.targetRequirement)},
        {"effect", cardEffectToString(definition.effect)},
        {"countsAsOwnTurnCard", definition.countsAsOwnTurnCard},
        {"rulesText", definition.rulesText}};
}

json playerCardsToJson(const PlayerCardState &state)
{
    return {
        {"deckCount", state.deck.size()},
        {"hand", state.hand},
        {"discardPile", state.discardPile},
        {"activePile", state.activePile},
        {"cardPlayedOnOwnTurn", state.cardPlayedOnOwnTurn},
        {"discardedThisTurn", state.discardedThisTurn},
        {"drawAtStartOfNextTurn", state.drawAtStartOfNextTurn},
        {"drawnThisTurn", state.drawnThisTurn}};
}

json cardStatesToJson(const GameState &game)
{
    return {
        {"White", playerCardsToJson(game.getCardState("White"))},
        {"Black", playerCardsToJson(game.getCardState("Black"))}};
}

json continuingEffectsToJson(const GameState &game)
{
    json effects = json::array();
    for (const ContinuingEffect &effect : game.getContinuingEffects())
    {
        effects.push_back({
            {"id", effect.id},
            {"cardId", effect.cardId},
            {"playedBy", effect.playedBy},
            {"targetSquare", effect.targetSquare},
            {"active", effect.active}});
    }
    return effects;
}

json cardCatalogToJson(const GameState &game)
{
    json catalog = json::array();
    for (const CardDefinition &definition : game.getCardManager().getDefinitions())
    {
        catalog.push_back(cardDefinitionToJson(definition));
    }
    return catalog;
}

json makeGameResponse(GameState &game, bool valid, const std::string &message)
{
    Board &board = game.getBoard();
    return {
        {"valid", valid},
        {"board", boardToJson(board)},
        {"turn", board.getTurn()},
        {"gameOver", board.isGameOver()},
        {"message", message},
        {"neutralSquares", game.getNeutralSquares()},
        {"warlordSquares", game.getWarlordSquares()},
        {"continuingEffects", continuingEffectsToJson(game)},
        {"cards", cardStatesToJson(game)},
        {"pendingCheckmate", game.getPendingCheckmatePlayer()}};
}

json makeMoveResponse(const json &body, GameState &game)
{
    const json move = body.value("move", json::object());
    const std::string fromSquare = move.value("from", json::object()).value("square", "");
    const std::string toSquare = move.value("to", json::object()).value("square", "");

    if (fromSquare.empty() || toSquare.empty())
    {
        return makeGameResponse(game, false, "Expected move square data.");
    }

    std::ostringstream capturedOutput;
    std::streambuf *previousBuffer = std::cout.rdbuf(capturedOutput.rdbuf());
    const std::string promotionPiece = body.value("promotion", "Queen");
    const bool moved = game.movePiece(fromSquare, toSquare, promotionPiece);
    std::cout.rdbuf(previousBuffer);

    const std::string message = trim(capturedOutput.str());
    const std::string responseMessage = message.empty() ? (moved ? "Move accepted." : "Invalid move.") : message;
    game.setLastMessage(responseMessage);
    return makeGameResponse(game, moved, responseMessage);
}

json makeLegalMovesResponse(const json &body, GameState &game)
{
    const std::string square = body.value("square", "");
    const std::vector<std::string> moves = game.getBoard().getLegalMoves(square);
    return {
        {"valid", true},
        {"square", square},
        {"moves", moves}};
}

json makeCardActionResponse(GameState &game, const CardResult &result)
{
    json response = makeGameResponse(game, result.success, result.message);
    response["cardResult"] = {
        {"success", result.success},
        {"message", result.message},
        {"cardId", result.cardId},
        {"targetSquare", result.targetSquare},
        {"drawnCardId", result.drawnCardId}};
    return response;
}

json makePlayCardResponse(const json &body, GameState &game)
{
    std::vector<std::string> fromSquares;
    for (const auto &square : body.value("fromSquares", json::array()))
    {
        if (square.is_string())
        {
            fromSquares.push_back(square.get<std::string>());
        }
    }

    CardAction action{
        body.value("cardId", ""),
        body.value("targetSquare", ""),
        body.value("fromSquare", ""),
        body.value("secondTargetSquare", ""),
        body.value("promotion", "Queen"),
        fromSquares};
    CardResult result = game.playCard(body.value("player", ""), action);
    return makeCardActionResponse(game, result);
}

json makeDiscardCardResponse(const json &body, GameState &game)
{
    CardResult result = game.discardCard(body.value("player", ""), body.value("cardId", ""));
    return makeCardActionResponse(game, result);
}

std::string getHeaderValue(const std::string &request, const std::string &headerName)
{
    std::string lowerRequest = request;
    std::string lowerHeaderName = headerName;
    std::transform(lowerRequest.begin(), lowerRequest.end(), lowerRequest.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    std::transform(lowerHeaderName.begin(), lowerHeaderName.end(), lowerHeaderName.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });

    size_t headerPosition = lowerRequest.find("\r\n" + lowerHeaderName + ":");
    if (headerPosition == std::string::npos)
    {
        return "";
    }

    size_t valueStart = request.find(':', headerPosition);
    size_t valueEnd = request.find("\r\n", valueStart);
    if (valueStart == std::string::npos || valueEnd == std::string::npos)
    {
        return "";
    }

    return trim(request.substr(valueStart + 1, valueEnd - valueStart - 1));
}

std::string toLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

std::string decodeChunkedBody(const std::string &body)
{
    std::string decoded;
    size_t position = 0;

    while (position < body.size())
    {
        size_t lineEnd = body.find("\r\n", position);
        if (lineEnd == std::string::npos)
        {
            break;
        }

        const std::string sizeText = body.substr(position, lineEnd - position);
        const size_t chunkSize = static_cast<size_t>(std::stoul(sizeText, nullptr, 16));
        if (chunkSize == 0)
        {
            break;
        }

        position = lineEnd + 2;
        if (position + chunkSize > body.size())
        {
            break;
        }

        decoded.append(body, position, chunkSize);
        position += chunkSize + 2;
    }

    return decoded;
}

std::string readRequest(SOCKET client)
{
    std::string request;
    char buffer[4096];

    while (request.find("\r\n\r\n") == std::string::npos)
    {
        int received = recv(client, buffer, sizeof(buffer), 0);
        if (received <= 0)
        {
            return request;
        }
        request.append(buffer, received);
        if (request.size() > MAX_REQUEST_SIZE)
        {
            return "";
        }
    }

    const std::string contentLengthText = getHeaderValue(request, "Content-Length");
    const std::string transferEncoding = toLower(getHeaderValue(request, "Transfer-Encoding"));
    const int contentLength = contentLengthText.empty() ? 0 : std::stoi(contentLengthText);
    const size_t bodyStart = request.find("\r\n\r\n") + 4;

    if (transferEncoding.find("chunked") != std::string::npos)
    {
        while (request.find("\r\n0\r\n\r\n", bodyStart) == std::string::npos)
        {
            int received = recv(client, buffer, sizeof(buffer), 0);
            if (received <= 0)
            {
                break;
            }
            request.append(buffer, received);
            if (request.size() > MAX_REQUEST_SIZE)
            {
                return "";
            }
        }
    }
    else
    {
        while (static_cast<int>(request.size() - bodyStart) < contentLength)
        {
            int received = recv(client, buffer, sizeof(buffer), 0);
            if (received <= 0)
            {
                break;
            }
            request.append(buffer, received);
            if (request.size() > MAX_REQUEST_SIZE)
            {
                return "";
            }
        }
    }

    return request;
}

void sendResponse(SOCKET client, int status, const std::string &body)
{
    const std::string statusText = status == 200 ? "OK" : status == 204 ? "No Content" : "Bad Request";
    std::ostringstream response;
    response << "HTTP/1.1 " << status << ' ' << statusText << "\r\n"
             << "Content-Type: application/json\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "Access-Control-Allow-Methods: GET,POST,OPTIONS\r\n"
             << "Access-Control-Allow-Headers: Content-Type\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body;

    const std::string responseText = response.str();
    size_t totalSent = 0;
    while (totalSent < responseText.size())
    {
        int sent = send(
            client,
            responseText.c_str() + totalSent,
            static_cast<int>(responseText.size() - totalSent),
            0);

        if (sent == SOCKET_ERROR)
        {
            break;
        }

        totalSent += static_cast<size_t>(sent);
    }
}

void handleClient(SOCKET client, GameState &game)
{
    try
    {
        const std::string request = readRequest(client);
        const size_t firstLineEnd = request.find("\r\n");
        const std::string firstLine = request.substr(0, firstLineEnd);

        if (firstLine.find("OPTIONS ") == 0)
        {
            sendResponse(client, 204, "{}");
        }
        else if (firstLine.find("GET /api/health ") == 0)
        {
            sendResponse(client, 200, json{{"ok", true}, {"service", "cpp-chess-backend"}}.dump());
        }
        else if (firstLine.find("GET /api/state ") == 0)
        {
            sendResponse(client, 200, makeGameResponse(game, true, game.getLastMessage()).dump());
        }
        else if (firstLine.find("GET /api/cards ") == 0)
        {
            sendResponse(client, 200, json{
                {"definitions", cardCatalogToJson(game)},
                {"players", cardStatesToJson(game)}}.dump());
        }
        else if (firstLine.find("POST /api/reset ") == 0)
        {
            game.reset();
            sendResponse(client, 200, makeGameResponse(game, true, game.getLastMessage()).dump());
        }
        else if (firstLine.find("POST /api/claim-checkmate ") == 0)
        {
            game.claimCheckmate();
            sendResponse(client, 200, makeGameResponse(game, true, game.getLastMessage()).dump());
        }
        else if (firstLine.find("POST /api/move ") == 0 ||
                 firstLine.find("POST /api/legal-moves ") == 0 ||
                 firstLine.find("POST /api/card/play ") == 0 ||
                 firstLine.find("POST /api/card/discard ") == 0 ||
                 firstLine.find("POST /api/card/draw ") == 0)
        {
            const size_t bodyStart = request.find("\r\n\r\n");
            std::string body = bodyStart == std::string::npos ? "" : request.substr(bodyStart + 4);
            const std::string transferEncoding = toLower(getHeaderValue(request, "Transfer-Encoding"));
            if (transferEncoding.find("chunked") != std::string::npos)
            {
                body = decodeChunkedBody(body);
            }
            const json requestBody = body.empty() ? json::object() : json::parse(body);

            if (firstLine.find("POST /api/move ") == 0)
            {
                sendResponse(client, 200, makeMoveResponse(requestBody, game).dump());
            }
            else if (firstLine.find("POST /api/legal-moves ") == 0)
            {
                sendResponse(client, 200, makeLegalMovesResponse(requestBody, game).dump());
            }
            else if (firstLine.find("POST /api/card/play ") == 0)
            {
                sendResponse(client, 200, makePlayCardResponse(requestBody, game).dump());
            }
            else if (firstLine.find("POST /api/card/discard ") == 0)
            {
                sendResponse(client, 200, makeDiscardCardResponse(requestBody, game).dump());
            }
            else
            {
                CardResult result = game.drawCardAsTurn(requestBody.value("player", ""));
                sendResponse(client, 200, makeCardActionResponse(game, result).dump());
            }
        }
        else
        {
            sendResponse(client, 400, json{{"valid", false}, {"message", "Route not found."}}.dump());
        }
    }
    catch (const std::exception &)
    {
        sendResponse(client, 400, json{{"valid", false}, {"message", "Malformed request."}}.dump());
    }

    shutdown(client, SD_SEND);
    closesocket(client);
}
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Could not create server socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(PORT);

    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
    {
        std::cerr << "Could not bind to port " << PORT << ".\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Could not listen on port " << PORT << ".\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "C++ chess backend listening on http://localhost:" << PORT << "\n";
    GameState game;

    while (true)
    {
        SOCKET client = accept(serverSocket, nullptr, nullptr);
        if (client != INVALID_SOCKET)
        {
            setsockopt(
                client,
                SOL_SOCKET,
                SO_RCVTIMEO,
                reinterpret_cast<const char *>(&CLIENT_TIMEOUT_MS),
                sizeof(CLIENT_TIMEOUT_MS));
            setsockopt(
                client,
                SOL_SOCKET,
                SO_SNDTIMEO,
                reinterpret_cast<const char *>(&CLIENT_TIMEOUT_MS),
                sizeof(CLIENT_TIMEOUT_MS));
            handleClient(client, game);
        }
    }
}
