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
#include "ChessPiece.hpp"

namespace
{
constexpr int PORT = 8787;
constexpr DWORD CLIENT_TIMEOUT_MS = 1000;
constexpr size_t MAX_REQUEST_SIZE = 64 * 1024;

struct MoveRequest
{
    std::string fromSquare;
    std::string toSquare;
    std::string promotionPiece;
};

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

std::string jsonEscape(const std::string &value)
{
    std::string escaped;

    for (char character : value)
    {
        if (character == '"' || character == '\\')
        {
            escaped += '\\';
        }

        if (character == '\n' || character == '\r')
        {
            escaped += ' ';
        }
        else
        {
            escaped += character;
        }
    }

    return escaped;
}

std::string findStringValue(const std::string &json, const std::string &key, size_t startAt = 0)
{
    const std::string quotedKey = "\"" + key + "\"";
    size_t keyPosition = json.find(quotedKey, startAt);
    if (keyPosition == std::string::npos)
    {
        return "";
    }

    size_t colonPosition = json.find(':', keyPosition + quotedKey.size());
    if (colonPosition == std::string::npos)
    {
        return "";
    }

    size_t quoteStart = json.find('"', colonPosition + 1);
    if (quoteStart == std::string::npos)
    {
        return "";
    }

    size_t quoteEnd = json.find('"', quoteStart + 1);
    if (quoteEnd == std::string::npos)
    {
        return "";
    }

    return json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
}

size_t findObjectPosition(const std::string &json, const std::string &key)
{
    const std::string quotedKey = "\"" + key + "\"";
    return json.find(quotedKey);
}

std::string findSquareInObject(const std::string &json, const std::string &objectKey)
{
    size_t objectPosition = findObjectPosition(json, objectKey);
    if (objectPosition == std::string::npos)
    {
        return "";
    }

    return findStringValue(json, "square", objectPosition);
}

std::vector<std::string> parseBoard(const std::string &json)
{
    std::vector<std::string> squares;
    size_t boardPosition = findObjectPosition(json, "board");
    if (boardPosition == std::string::npos)
    {
        return squares;
    }

    size_t arrayStart = json.find('[', boardPosition);
    if (arrayStart == std::string::npos)
    {
        return squares;
    }

    int depth = 0;
    bool inString = false;
    bool escaping = false;
    std::string currentString;

    for (size_t i = arrayStart; i < json.size(); i++)
    {
        char character = json[i];

        if (inString)
        {
            if (escaping)
            {
                currentString += character;
                escaping = false;
            }
            else if (character == '\\')
            {
                escaping = true;
            }
            else if (character == '"')
            {
                squares.push_back(currentString);
                currentString.clear();
                inString = false;
            }
            else
            {
                currentString += character;
            }
            continue;
        }

        if (character == '"')
        {
            inString = true;
            continue;
        }

        if (character == '[')
        {
            depth++;
        }
        else if (character == ']')
        {
            depth--;
            if (depth == 0)
            {
                break;
            }
        }
    }

    return squares;
}

MoveRequest parseMoveRequest(const std::string &body)
{
    MoveRequest request;
    request.fromSquare = findSquareInObject(body, "from");
    request.toSquare = findSquareInObject(body, "to");
    request.promotionPiece = findStringValue(body, "promotion");
    return request;
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

std::string boardToJson(Board &board)
{
    std::ostringstream json;
    json << '[';

    for (int row = 0; row < 8; row++)
    {
        if (row > 0)
        {
            json << ',';
        }

        json << '[';
        for (int col = 0; col < 8; col++)
        {
            if (col > 0)
            {
                json << ',';
            }

            Pieces *piece = board.getPieceAt(frontendSquareFor(row, col));
            char symbol = symbolForPiece(piece);
            json << '"';
            if (symbol != '\0')
            {
                json << symbol;
            }
            json << '"';
        }
        json << ']';
    }

    json << ']';
    return json.str();
}

std::unique_ptr<Pieces> createPiece(char symbol, const std::string &square)
{
    const bool white = std::isupper(static_cast<unsigned char>(symbol));
    const char lowerSymbol = static_cast<char>(std::tolower(static_cast<unsigned char>(symbol)));
    const std::string color = white ? "White" : "Black";

    if (lowerSymbol == 'p')
    {
        return std::make_unique<Pawn>(color, false, square);
    }
    if (lowerSymbol == 'n')
    {
        return std::make_unique<Knight>(color, false, square);
    }
    if (lowerSymbol == 'b')
    {
        return std::make_unique<Bishop>(color, false, square);
    }
    if (lowerSymbol == 'r')
    {
        return std::make_unique<Rook>(color, false, square);
    }
    if (lowerSymbol == 'q')
    {
        return std::make_unique<Queen>(color, false, square);
    }
    if (lowerSymbol == 'k')
    {
        return std::make_unique<King>(color, false, square);
    }

    return nullptr;
}

bool populateBoard(const std::vector<std::string> &frontendBoard, Board &board, std::vector<std::unique_ptr<Pieces>> &pieces)
{
    if (frontendBoard.size() != 64)
    {
        return false;
    }

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            const std::string &symbol = frontendBoard[static_cast<size_t>(row * 8 + col)];
            if (symbol.empty())
            {
                continue;
            }

            std::string square = frontendSquareFor(row, col);
            auto piece = createPiece(symbol[0], square);
            if (!piece)
            {
                return false;
            }

            board.placePiece(piece.get(), piece->getType(), square);
            pieces.push_back(std::move(piece));
        }
    }

    return true;
}

const std::vector<std::string> STARTING_BOARD = {
    "r", "n", "b", "q", "k", "b", "n", "r",
    "p", "p", "p", "p", "p", "p", "p", "p",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "P", "P", "P", "P", "P", "P", "P", "P",
    "R", "N", "B", "Q", "K", "B", "N", "R"};

class GameState
{
private:
    std::unique_ptr<Board> board;
    std::vector<std::unique_ptr<Pieces>> pieces;
    std::string lastMessage;

public:
    GameState()
    {
        reset();
    }

    void reset()
    {
        board = std::make_unique<Board>();
        pieces.clear();

        std::ostringstream setupOutput;
        std::streambuf *previousBuffer = std::cout.rdbuf(setupOutput.rdbuf());
        populateBoard(STARTING_BOARD, *board, pieces);
        std::cout.rdbuf(previousBuffer);
        lastMessage = "New game started. White to move.";
    }

    Board &getBoard()
    {
        return *board;
    }

    const std::string &getLastMessage() const
    {
        return lastMessage;
    }

    void setLastMessage(const std::string &message)
    {
        lastMessage = message;
    }
};

std::string makeGameResponse(GameState &game, bool valid, const std::string &message)
{
    Board &board = game.getBoard();
    std::ostringstream response;
    response << "{\"valid\":" << (valid ? "true" : "false")
             << ",\"board\":" << boardToJson(board)
             << ",\"turn\":\"" << board.getTurn() << "\""
             << ",\"gameOver\":" << (board.isGameOver() ? "true" : "false")
             << ",\"message\":\"" << jsonEscape(message) << "\"}";
    return response.str();
}

std::string makeMoveResponse(const std::string &body, GameState &game)
{
    MoveRequest request = parseMoveRequest(body);

    if (request.fromSquare.empty() || request.toSquare.empty())
    {
        return makeGameResponse(game, false, "Expected move square data.");
    }

    std::ostringstream capturedOutput;
    std::streambuf *previousBuffer = std::cout.rdbuf(capturedOutput.rdbuf());
    const std::string promotionPiece = request.promotionPiece.empty() ? "Queen" : request.promotionPiece;
    const bool moved = game.getBoard().movePiece(request.fromSquare, request.toSquare, promotionPiece);
    std::cout.rdbuf(previousBuffer);

    const std::string message = trim(capturedOutput.str());
    const std::string responseMessage = message.empty() ? (moved ? "Move accepted." : "Invalid move.") : message;
    game.setLastMessage(responseMessage);
    return makeGameResponse(game, moved, responseMessage);
}

std::string makeLegalMovesResponse(const std::string &body, GameState &game)
{
    const std::string square = findStringValue(body, "square");
    const std::vector<std::string> moves = game.getBoard().getLegalMoves(square);
    std::ostringstream response;
    response << "{\"valid\":true,\"square\":\"" << jsonEscape(square) << "\",\"moves\":[";

    for (size_t index = 0; index < moves.size(); index++)
    {
        if (index > 0)
        {
            response << ',';
        }

        response << '"' << jsonEscape(moves[index]) << '"';
    }

    response << "]}";
    return response.str();
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
            sendResponse(client, 200, "{\"ok\":true,\"service\":\"cpp-chess-backend\"}");
        }
        else if (firstLine.find("GET /api/state ") == 0)
        {
            sendResponse(client, 200, makeGameResponse(game, true, game.getLastMessage()));
        }
        else if (firstLine.find("POST /api/reset ") == 0)
        {
            game.reset();
            sendResponse(client, 200, makeGameResponse(game, true, game.getLastMessage()));
        }
        else if (firstLine.find("POST /api/move ") == 0 ||
                 firstLine.find("POST /api/legal-moves ") == 0)
        {
            const size_t bodyStart = request.find("\r\n\r\n");
            std::string body = bodyStart == std::string::npos ? "" : request.substr(bodyStart + 4);
            const std::string transferEncoding = toLower(getHeaderValue(request, "Transfer-Encoding"));
            if (transferEncoding.find("chunked") != std::string::npos)
            {
                body = decodeChunkedBody(body);
            }

            if (firstLine.find("POST /api/move ") == 0)
            {
                sendResponse(client, 200, makeMoveResponse(body, game));
            }
            else
            {
                sendResponse(client, 200, makeLegalMovesResponse(body, game));
            }
        }
        else
        {
            sendResponse(client, 400, "{\"valid\":false,\"message\":\"Route not found.\"}");
        }
    }
    catch (const std::exception &)
    {
        sendResponse(client, 400, "{\"valid\":false,\"message\":\"Malformed request.\"}");
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
