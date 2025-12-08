class WebChat {
    constructor() {
        this.sessionToken = localStorage.getItem('chat_session_token') || '';
        this.currentUser = localStorage.getItem('chat_current_user') || null;
        this.currentChat = null;
        this.chats = [];
        this.users = [];
        this.pollingInterval = null;
        
        // Try auto-login if session exists
        if (this.sessionToken && this.currentUser) {
            this.tryAutoLogin();
        }
    }

    // API calls
    async apiCall(endpoint, options = {}) {
        const defaultOptions = {
            headers: {
                'Content-Type': 'application/json',
            }
        };

        if (this.sessionToken) {
            defaultOptions.headers['Authorization'] = `Bearer ${this.sessionToken}`;
        }

        const finalOptions = { ...defaultOptions, ...options };
        
        try {
            const response = await fetch(endpoint, finalOptions);
            let data;
            
            // Check Content-Type before parsing
            const contentType = response.headers.get('content-type');
            if (contentType && contentType.includes('application/json')) {
                data = await response.json();
            } else {
                data = await response.text();
            }
            
            if (!response.ok) {
                throw new Error(data.message || data || 'API error');
            }
            
            return data;
        } catch (error) {
            console.error('API call failed:', error);
            throw error;
        }
    }

    // Auto-login functionality
    async tryAutoLogin() {
        if (this.sessionToken && this.currentUser) {
            try {
                // Check if session is still valid
                await this.apiCall('/api/validate_session');
                this.showChatScreen();
                this.startPolling();
                console.log('Auto-login successful');
            } catch (error) {
                // Session invalid, clear storage
                this.logout();
                console.log('Auto-login failed, session expired');
            }
        }
    }

    // Authentication
    async login(username, password) {
        try {
            const data = await this.apiCall('/api/login', {
                method: 'POST',
                body: JSON.stringify({ username, password })
            });
            
            this.sessionToken = data.session_token;
            this.currentUser = username;
            this.currentUserId = data.user_id;  // ← ДОБАВЛЕНО
            
            // Save to localStorage for persistence
            localStorage.setItem('chat_session_token', this.sessionToken);
            localStorage.setItem('chat_current_user', this.currentUser);
            localStorage.setItem('chat_current_user_id', this.currentUserId);  // ← ДОБАВЛЕНО
            
            this.showChatScreen();
            this.startPolling();
            this.showMessage('Login successful!', 'success');
        } catch (error) {
            this.showMessage(error.message, 'error');
        }
    }

    async register(username, password, email) {
        try {
            const data = await this.apiCall('/api/register', {
                method: 'POST',
                body: JSON.stringify({ username, password, email })
            });
            
            this.showMessage('Registration successful! Please login.', 'success');
            this.showLogin();
        } catch (error) {
            this.showMessage(error.message, 'error');
        }
    }

    logout() {
        this.sessionToken = '';
        this.currentUser = null;
        this.currentChat = null;
        this.chats = [];
        this.stopPolling();
        
        // Clear localStorage
        localStorage.removeItem('chat_session_token');
        localStorage.removeItem('chat_current_user');
        
        this.clearChatInterface();
        this.showLoginScreen();
    }

    // Chat management
    async loadChats() {
        try {
            const data = await this.apiCall('/api/chats');
            console.log('Chats API response:', data);
            
            // Handle different response formats
            if (data && Array.isArray(data.chats)) {
                this.chats = data.chats;
            } else if (Array.isArray(data)) {
                this.chats = data;
            } else {
                this.chats = [];
                console.warn('Unexpected chats response format:', data);
            }
            
            this.renderChats();
            
            // Reset interface if no chats
            if (this.chats.length === 0) {
                this.currentChat = null;
                this.clearChatInterface();
            }
            
        } catch (error) {
            console.error('Failed to load chats:', error);
            this.chats = [];
            this.renderChats();
            this.currentChat = null;
            this.clearChatInterface();
        }
    }
    
    async createChat(chatName, isPublic = true) {
        console.log('Creating chat:', chatName, 'Public:', isPublic);
        try {
            const data = await this.apiCall('/api/chats/create_with_privacy', {
                method: 'POST',
                body: JSON.stringify({ 
                    chat_name: chatName,
                    is_public: isPublic 
                })
            });
            
            console.log('Chat creation response:', data);
            
            this.hideCreateChat();
            
            // Создаем локальный объект чата
            const newChat = {
                chat_id: data.chat_id,
                chat_name: chatName,
                chat_type: "group", 
                member_count: 1,
                is_public: data.is_public
            };
            
            // Добавляем в начало списка
            this.chats.unshift(newChat);
            this.renderChats();
            
            // Сбрасываем текущий чат
            this.currentChat = null;
            this.clearChatInterface();
            
            const typeText = isPublic ? 'public' : 'private';
            this.showMessage(`${typeText} chat created successfully! ID: ${data.chat_id}`, 'success');
            
            // Перезагружаем чаты с сервера
            setTimeout(async () => {
                await this.loadChats();
            }, 500);
            
        } catch (error) {
            console.error('Failed to create chat:', error);
            this.showMessage('Failed to create chat: ' + error.message, 'error');
        }
    }

    async loadUsers() {
        const usersList = document.getElementById('users-list');
        if (!usersList) return;
        
        // Temporary placeholder
        usersList.innerHTML = `
            <div class="user-item" style="color: #999; font-style: italic;">
                User list functionality coming soon...
            </div>
        `;
    }

    async selectChat(chatId) {
        // Don't do anything if selecting the same chat
        if (this.currentChat && this.currentChat.chat_id === chatId) {
            return;
        }
        
        this.currentChat = this.chats.find(chat => chat.chat_id === chatId);
        if (this.currentChat) {
            const currentChatName = document.getElementById('current-chat-name');
            const chatActions = document.getElementById('chat-actions');
            const messageInput = document.getElementById('message-input');
            const sendButton = document.getElementById('send-button');
            
            if (currentChatName) currentChatName.textContent = this.currentChat.chat_name;
            if (chatActions) chatActions.style.display = 'block';
            if (messageInput) {
                messageInput.disabled = false;
                messageInput.placeholder = 'Type a message...';
            }
            if (sendButton) sendButton.disabled = false;
            
            // Update active chat in UI
            document.querySelectorAll('.chat-item').forEach(item => {
                item.classList.remove('active');
            });
            const selectedChat = document.querySelector(`[data-chat-id="${chatId}"]`);
            if (selectedChat) selectedChat.classList.add('active');
            
            await this.loadMessages();
        } else {
            console.error('Chat not found:', chatId);
        }
    }

    async loadMessages() {
        if (!this.currentChat) {
            console.log('No chat selected, skipping message load');
            return;
        }
        
        try {
            const data = await this.apiCall(`/api/chats/${this.currentChat.chat_id}/messages`);
            this.renderMessages(data.messages || []);
        } catch (error) {
            console.error('Failed to load messages:', error);
            // Show empty messages on error
            this.renderMessages([]);
        }
    }

    async sendMessage() {
        const input = document.getElementById('message-input');
        const content = input.value.trim();
        
        if (!content || !this.currentChat) return;
        
        try {
            await this.apiCall('/api/messages', {
                method: 'POST',
                body: JSON.stringify({
                    chat_id: this.currentChat.chat_id,
                    content: content
                })
            });
            
            input.value = '';
            await this.loadMessages(); // Reload to see the new message
        } catch (error) {
            this.showMessage('Failed to send message: ' + error.message, 'error');
        }
    }

    // Search functionality
    async searchChat() {
        const searchInput = document.getElementById('chat-search');
        const chatId = parseInt(searchInput.value.trim());
        
        if (!chatId || isNaN(chatId)) {
            this.showMessage('Please enter a valid chat ID', 'error');
            return;
        }
        
        try {
            const data = await this.apiCall('/api/chats/search', {
                method: 'POST',
                body: JSON.stringify({ chat_id: chatId })
            });
            
            this.showSearchResult(data);
            
        } catch (error) {
            this.showMessage('Chat not found: ' + error.message, 'error');
            this.closeSearchResult();
        }
    }

    showSearchResult(chatData) {
        const chatList = document.getElementById('chat-list');
        if (!chatList) return;
        
        // Создаем элемент результата поиска
        const resultElement = document.createElement('div');
        resultElement.className = 'search-result';
        
        let privacyBadge = '';
        if (!chatData.is_public) {
            privacyBadge = '<span class="privacy-badge private">🔒 Private</span>';
        }
        
        resultElement.innerHTML = `
            <h4>${this.escapeHtml(chatData.chat_name)} ${privacyBadge}</h4>
            <p>ID: ${chatData.chat_id} • Members: ${chatData.member_count}</p>
            <button onclick="chatApp.joinChat(${chatData.chat_id})">Join Chat</button>
        `;
        
        // Вставляем в начало списка чатов
        chatList.insertBefore(resultElement, chatList.firstChild);
    }
    

    async joinChat(chatId) {
        try {
            const data = await this.apiCall('/api/chats/join', {
                method: 'POST',
                body: JSON.stringify({ chat_id: chatId })
            });
            
            this.showMessage('Successfully joined chat!', 'success');
            
            // Закрываем результат поиска
            this.closeSearchResult();
            
            // Перезагружаем список чатов
            await this.loadChats();
            
        } catch (error) {
            this.showMessage(error.message, 'error');
        }
    }

    async sendInvite() {
        const userId = parseInt(document.getElementById('invite-user-id').value);
        
        if (!userId || isNaN(userId)) {
            chatApp.showMessage('Please enter a valid user ID', 'error');
            return;
        }
        
        if (!chatApp.currentChat) {
            chatApp.showMessage('No chat selected', 'error');
            return;
        }
        
        try {
            await chatApp.apiCall(`/api/chats/${chatApp.currentChat.chat_id}/invite`, {
                method: 'POST',
                body: JSON.stringify({ user_id: userId })
            });
            
            chatApp.showMessage(`User ${userId} invited successfully!`, 'success');
            chatApp.hideInviteUser();
            
        } catch (error) {
            chatApp.showMessage('Failed to invite user: ' + error.message, 'error');
        }
    }

    async addUserToChat(userId) {
        if (!this.currentChat) return;
        
        try {
            await this.apiCall(`/api/chats/${this.currentChat.chat_id}/add_user`, {
                method: 'POST',
                body: JSON.stringify({ user_id: userId })
            });
            
            this.hideInviteUser();
            this.showMessage('User added to chat!', 'success');
        } catch (error) {
            this.showMessage(error.message, 'error');
        }
    }

    // UI Rendering
    renderChats() {
        const chatList = document.getElementById('chat-list');
        if (!chatList) {
            console.error('chat-list element not found!');
            return;
        }
        
        chatList.innerHTML = '';
        
        if (this.chats.length === 0) {
            chatList.innerHTML = '<div class="no-chats">No chats yet. Create one!</div>';
            return;
        }
        
        this.chats.forEach(chat => {
            const chatElement = document.createElement('div');
            chatElement.className = 'chat-item';
            chatElement.setAttribute('data-chat-id', chat.chat_id);
            chatElement.innerHTML = `
                <div><strong>${chat.chat_name}</strong></div>
                <small>ID: ${chat.chat_id} • ${chat.member_count} members</small>
            `;
            chatElement.onclick = () => this.selectChat(chat.chat_id);
            chatList.appendChild(chatElement);
        });
    }

    renderMessages(messages) {
        const messagesContainer = document.getElementById('messages');
        if (!messagesContainer) {
            console.error('Messages container not found');
            return;
        }
        
        // Clear container
        messagesContainer.innerHTML = '';
        
        // If no messages, show placeholder
        if (!messages || messages.length === 0) {
            messagesContainer.innerHTML = `
                <div style="text-align: center; padding: 2rem; color: #999;">
                    <p>No messages yet. Start the conversation!</p>
                </div>
            `;
            return;
        }
        
        // Render messages
        messages.forEach(msg => {
            const messageElement = document.createElement('div');
            messageElement.className = `message-item ${msg.sender_name === this.currentUser ? 'own' : 'other'}`;
            messageElement.innerHTML = `
                <div class="message-sender">${msg.sender_name}</div>
                <div class="message-content">${this.escapeHtml(msg.content)}</div>
                <div class="message-time">${msg.timestamp}</div>
            `;
            messagesContainer.appendChild(messageElement);
        });
        
        // Scroll to bottom
        messagesContainer.scrollTop = messagesContainer.scrollHeight;
    }

    // UI Navigation
    showLoginScreen() {
        document.getElementById('login-screen').style.display = 'flex';
        document.getElementById('chat-screen').style.display = 'none';
    }

    showChatScreen() {
        document.getElementById('login-screen').style.display = 'none';
        document.getElementById('chat-screen').style.display = 'block';
        
        // Отображаем информацию о пользователе
        const currentUserElement = document.getElementById('current-user');
        const currentUserIdElement = document.getElementById('current-user-id');
        
        if (currentUserElement) {
            currentUserElement.textContent = this.currentUser || '';
        }
        
        if (currentUserIdElement) {
            currentUserIdElement.textContent = this.currentUserId || '';
        }
        
        // Скрываем кнопку Clear при загрузке
        const clearBtn = document.getElementById('clear-search-btn');
        if (clearBtn) {
            clearBtn.style.display = 'none';
        }
        
        // Очищаем результаты поиска
        this.closeSearchResult();
        
        // Reset state when showing chat screen
        this.currentChat = null;
        this.clearChatInterface();
        
        this.loadChats();
        this.loadUsers();
    }

    showSearchResult(chatData) {
        const chatList = document.getElementById('chat-list');
        if (!chatList) return;
        
        // Проверяем, не существует ли уже результат поиска
        this.removeSearchResult();
        
        // Создаем элемент результата поиска
        const resultElement = document.createElement('div');
        resultElement.className = 'search-result';
        resultElement.id = 'search-result-item';  // Добавляем ID для поиска
        
        let privacyBadge = '';
        if (!chatData.is_public) {
            privacyBadge = '<span class="privacy-badge private">🔒 Private</span>';
        } else {
            privacyBadge = '<span class="privacy-badge public">🌐 Public</span>';
        }
        
        resultElement.innerHTML = `
            <div style="display: flex; justify-content: space-between; align-items: start;">
                <div>
                    <h4>${this.escapeHtml(chatData.chat_name)} ${privacyBadge}</h4>
                    <p>ID: ${chatData.chat_id} • Members: ${chatData.member_count}</p>
                </div>
                <button onclick="chatApp.closeSearchResult()" style="background: transparent; border: none; color: #999; cursor: pointer; font-size: 18px;">×</button>
            </div>
            <button onclick="chatApp.joinChat(${chatData.chat_id})" style="margin-top: 8px;">Join Chat</button>
        `;
        
        // Вставляем в начало списка чатов
        chatList.insertBefore(resultElement, chatList.firstChild);
        
        // Показываем кнопку Clear
        const clearBtn = document.getElementById('clear-search-btn');
        if (clearBtn) {
            clearBtn.style.display = 'block';
        }
    }
    
    // Метод для закрытия результата поиска
    closeSearchResult() {
        this.removeSearchResult();
        
        // Очищаем поле поиска
        const searchInput = document.getElementById('chat-search');
        if (searchInput) {
            searchInput.value = '';
        }
        
        // Скрываем кнопку Clear
        const clearBtn = document.getElementById('clear-search-btn');
        if (clearBtn) {
            clearBtn.style.display = 'none';
        }
    }
    
    // Метод для удаления результата поиска
    removeSearchResult() {
        const existingResult = document.getElementById('search-result-item');
        if (existingResult) {
            existingResult.remove();
        }
    }

    showRegister() {
        document.getElementById('login-form').style.display = 'none';
        document.getElementById('register-form').style.display = 'block';
    }

    showLogin() {
        document.getElementById('register-form').style.display = 'none';
        document.getElementById('login-form').style.display = 'block';
    }

    showCreateChat() {
        document.getElementById('create-chat-modal').style.display = 'flex';
    }

    hideCreateChat() {
        document.getElementById('create-chat-modal').style.display = 'none';
        document.getElementById('new-chat-name').value = '';
    }

    showInviteUser() {
        if (!this.currentChat) {
            this.showMessage('Please select a chat first', 'error');
            return;
        }
        
        const inviteModal = document.getElementById('invite-user-modal');
        if (inviteModal) {
            inviteModal.style.display = 'flex';
        } else {
            console.error('Invite user modal not found');
            this.showMessage('Invite feature not available', 'error');
        }
    }

    hideInviteUser() {
        const inviteModal = document.getElementById('invite-user-modal');
        if (inviteModal) {
            inviteModal.style.display = 'none';
        }
    }

    // Utility
    showMessage(text, type) {
        const messageEl = document.getElementById('auth-message');
        messageEl.textContent = text;
        messageEl.className = `message ${type}`;
        messageEl.style.display = 'block';
        
        setTimeout(() => {
            messageEl.style.display = 'none';
        }, 5000);
    }

    escapeHtml(unsafe) {
        return unsafe
            .replace(/&/g, "&amp;")
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/"/g, "&quot;")
            .replace(/'/g, "&#039;");
    }

    clearChatInterface() {
        const messagesContainer = document.getElementById('messages');
        const currentChatName = document.getElementById('current-chat-name');
        const chatActions = document.getElementById('chat-actions');
        const messageInput = document.getElementById('message-input');
        const sendButton = document.getElementById('send-button');
        
        if (messagesContainer) messagesContainer.innerHTML = '';
        if (currentChatName) currentChatName.textContent = 'Select a Chat';
        if (chatActions) chatActions.style.display = 'none';
        if (messageInput) {
            messageInput.disabled = true;
            messageInput.value = '';
            messageInput.placeholder = 'Select a chat to start messaging...';
        }
        if (sendButton) sendButton.disabled = true;
        
        // Reset active chats in UI
        document.querySelectorAll('.chat-item').forEach(item => {
            item.classList.remove('active');
        });
    }

    // Polling for new messages
    startPolling() {
        this.pollingInterval = setInterval(() => {
            if (this.currentChat) {
                this.loadMessages();
            }
            // Don't try to load messages if no chat selected
        }, 2000); // Poll every 2 seconds
    }

    stopPolling() {
        if (this.pollingInterval) {
            clearInterval(this.pollingInterval);
            this.pollingInterval = null;
        }
    }
}

// Global chat instance
const chatApp = new WebChat();

// Global functions for HTML onclick handlers
function login() {
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    chatApp.login(username, password);
}


function register() {
    const username = document.getElementById('reg-username').value;
    const email = document.getElementById('reg-email').value;
    const password = document.getElementById('reg-password').value;
    chatApp.register(username, password, email);
}

function showRegister() {
    chatApp.showRegister();
}

function showLogin() {
    chatApp.showLogin();
}

function logout() {
    chatApp.logout();
}

function sendMessage() {
    chatApp.sendMessage();
}

function showCreateChat() {
    chatApp.showCreateChat();
}

function createChat() {
    const chatName = document.getElementById('new-chat-name').value;
    const isPublic = document.querySelector('input[name="chat-privacy"]:checked').value === 'public';
    
    if (chatName.trim()) {
        chatApp.createChat(chatName, isPublic);
    }
}

function hideCreateChat() {
    chatApp.hideCreateChat();
}

function inviteUser() {
    chatApp.showInviteUser();
}

function addUserToChat() {
    const userId = document.getElementById('user-select').value;
    chatApp.addUserToChat(parseInt(userId));
}

function hideInviteUser() {
    chatApp.hideInviteUser();
}

function searchChat() {
    chatApp.searchChat();
}

function clearSearch() {
    chatApp.closeSearchResult();
}

// Enter key handlers
document.addEventListener('DOMContentLoaded', function() {
    // Login form enter key
    document.getElementById('password').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') login();
    });
    
    // Register form enter key
    document.getElementById('reg-password').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') register();
    });
    
    // Message input enter key
    document.getElementById('message-input').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') sendMessage();
    });
    
    // New chat name enter key
    document.getElementById('new-chat-name').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') createChat();
    });
    
    // Search input enter key
    document.getElementById('chat-search').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') searchChat();
    });
});