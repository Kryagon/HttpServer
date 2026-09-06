#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
constexpr int BACKLOG = 10;
constexpr int BUFFER_SIZE = 65536;
int PORT = 8080;
std::string read_file(const std::string & path){
	std::ifstream file(path, std::ios::binary);
	if(!file.is_open())return "";
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}
std::string get_content_type(const std::string & path){
	size_t dot_pos = path.rfind('.');
	if(dot_pos == std::string::npos || dot_pos == 0)return "text/plain; charset=utf-8";
	std::string ext = path.substr(dot_pos);
	for(char & c:ext)c = tolower(c);
	if(ext == ".html" || ext == ".htm")return "text/html; charset=utf-8";
	if(ext == ".css")return "text/css; charset=utf-8";
	if(ext == ".js")return "application/javascript; charset=utf-8";
	if(ext == ".png")return "image/png";
	if(ext == ".jpg" || ext == ".jpeg")return "image/jpeg";
	if(ext == ".gif")return "image/gif";
	if(ext == ".ico")return "image/x-icon";
	return "text/plain; charset=utf-8";
}
std::string build_response(int status_code, const std::string & status_text, const std::string & content_type, const std::string & body){
	std::string response;
	response += "HTTP/1.1 " + std::to_string(status_code) + " " + status_text + "\r\n";
	response += "Content-Type: " + content_type + "\r\n";
	response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += body;
	return response;
}
std::string get_time(){
	time_t now = time(nullptr);
	struct tm * local_time = localtime(&now);
	char buffer[80];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);
	return std::string(buffer);
}
SOCKET create_server_socket(int port){
	WSADATA wsadata;
	if(WSAStartup(MAKEWORD(2, 2), &wsadata) != 0){
		std::cerr << "WSAStartup failed" << std::endl;
		return INVALID_SOCKET;
	}
	SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd == INVALID_SOCKET){
		std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return INVALID_SOCKET;
	}
	int opt = 1;
	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0){
		std::cerr << "setsockopt failed" << std::endl;
		closesocket(server_fd);
		WSACleanup();
		return INVALID_SOCKET;
	}
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
		std::cerr << "bind failed on port " << port << ": " << WSAGetLastError() << std::endl;
		closesocket(server_fd);
		WSACleanup();
		return INVALID_SOCKET;
	}
	if(listen(server_fd, BACKLOG) < 0){
		std::cerr << "listen failed" << std::endl;
		closesocket(server_fd);
		WSACleanup();
		return INVALID_SOCKET;
	}
	return server_fd;
}
void handle_client(SOCKET client_fd){
	char buffer[BUFFER_SIZE] = {0};
	int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
	if(bytes <= 0){
		closesocket(client_fd);
		return;
	}
	std::string request(buffer);
	std::istringstream iss(request);
	std::string method, path, version;
	iss >> method >> path >> version;
	std::cout << '[' << get_time() << "] " << method << ' ' << path << std::endl;
	if(path == "/")path = "/index.html";
	size_t qpos = path.find('?');
	if(qpos != std::string::npos)path = path.substr(0, qpos);
	std::string file_path = "." + path;
	std::string body = read_file(file_path);
	std::string response;
	if(body.empty()){
		std::string not_found = "<html><body><h1>404 - File Not Found</h1><p>Request: " + path + "</p></body></html>";
		response = build_response(404, "Not Found", get_content_type(path), not_found);
		std::cout << "  -> 404 Not Found" << std::endl;
	}
	else{
		response = build_response(200, "OK", get_content_type(path), body);
		std::cout << "  -> 200 OK (" << body.size() << " bytes)" << std::endl;
	}
	send(client_fd, response.c_str(), response.size(), 0);
	closesocket(client_fd);
}
int main(int argc, char * argv[]){
	if(argc >= 2)PORT = std::stoi(argv[1]);
	SOCKET server_fd = create_server_socket(PORT);
	if(server_fd == INVALID_SOCKET){
		std::cerr << "Failed to create server socket" << std::endl;
		return -1;
	}
	std::cout << "========================================" << std::endl;
	std::cout << "  HttpServer v1.0" << std::endl;
	std::cout << "  Server listening on port " << PORT << std::endl;
	std::cout << "  Try: http://127.0.0.1:" << PORT << std::endl;
	std::cout << "  Try: http://localhost:" << PORT << std::endl;
	std::cout << "========================================" << std::endl;
	std::cout << "  Press Ctrl+C to stop" << std::endl;
	std::cout << std::endl;
	while(true){
		sockaddr_in client_addr;
		int addr_len = sizeof(client_addr);
		SOCKET client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
		if(client_fd == INVALID_SOCKET){
			std::cerr << "accept  failed: " << WSAGetLastError() << std::endl;
			continue;
		}
		handle_client(client_fd);
	}
	return 0;
}
