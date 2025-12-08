class WebChat {
    constructor() {
        this.currentUserId = localStorage.getItem('chat_current_user_id') || null;
        this.currentUser = localStorage.getItem('chat_current_user') || null;
        this.currentChat = null;
        this.chats = [];
        this.pollingInterval = null;
    }

    // API calls
    async apiCall(endpoint, options = {}) {
        const defaultOptions = {
            headers: {
                'Content-Type': 'application/json',
            }
        };

        if (this.currentUserId) {
            defaultOptions.headers['Authorization'] = `Basic ${this.currentUserId}:dummy`;
        }

        const finalOptions = { ...defaultOptions, ...options };
        
        try {
            const response = await fetch(endpoint, finalOptions);
            let data;
            
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

    // Authentication
    async login(username, password) {
        try {
            const data = await this.apiCall('/api/login', {
                method: 'POST',
                headers: {}, // No auth for login
                body: JSON.stringify({ username, password })
            });
            
            this.currentUserId = data.user_id;
            this.currentUser = data.username;
            
            localStorage.setItem('chat_current_user_id', this.currentUserId);
            localStorage.setItem('chat_current_user', this.currentUser);
            
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
        this.currentUserId = null;
        this.currentUser = null;
        this.currentChat = null;
        this.chats = [];
        this.stopPolling();
        
        localStorage.removeItem('chat_current_user_id');
        localStorage.removeItem('chat_current_user');
        
        this.clearChatInterface();
        this.showLoginScreen();
    }

    // Chat management
    async loadChats() {
        try {
            const data = await this.apiCall('/api/chats');
            
            if (data && Array.isArray(data.chats)) {
                this.chats = data.chats;
            } else {
                this.chats = [];
            }
            
            this.renderChats();
            
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
    
    async createChat(chatName) {
        try {
            const data = await this.apiCall('/api/chats/create', {
                method: 'POST',
                body: JSON.stringify({ chat_name: chatName })
            });
            
            this.hideCreateChat();
            
            const newChat = {
                chat_id: data.chat_id,
                chat_name: chatName,
                member_count: 1
            };
            
            this.chats.unshift(newChat);
            this.renderChats();
            
            this.currentChat = null;
            this.clearChatInterface();
            
            this.showMessage(`Chat created successfully! ID: ${data.chat_id}`, 'success');
            
            setTimeout(async () => {
                await this.loadChats();
            }, 500);
            
        } catch (error) {
            this.showMessage('Failed to create chat: ' + error.message, 'error');
        }
    }

    async selectChat(chatId) {
        if (this.currentChat && this.currentChat.chat_id === chatId) {
            return;
        }
        
        this.currentChat = this.chats.find(chat => chat.chat_id === chatId);
        if (this.currentChat) {
            const currentChatName = document.getElementById('current-chat-name');
            const messageInput = document.getElementById('message-input');
            const sendButton = document.getElementById('send-button');
            
            if (currentChatName) currentChatName.textContent = this.currentChat.chat_name;
            if (messageInput) {
                messageInput.disabled = false;
                messageInput.placeholder = 'Type a message...';
            }
            if (sendButton) sendButton.disabled = false;
            
            document.querySelectorAll('.chat-item').forEach(item => {
                item.classList.remove('active');
            });
            const selectedChat = document.querySelector(`[data-chat-id="${chatId}"]`);
            if (selectedChat) selectedChat.classList.add('active');
            
            await this.loadMessages();
        }
    }

    async loadMessages() {
        if (!this.currentChat) return;
        
        try {
            const data = await this.apiCall(`/api/chats/${this.currentChat.chat_id}/messages`);
            this.renderMessages(data.messages || []);
        } catch (error) {
            console.error('Failed to load messages:', error);
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
            await this.loadMessages();
        } catch (error) {
            this.showMessage('Failed to send message: ' + error.message, 'error');
        }
    }

    // Search functionality (как в финальной версии - с крестиком, без кнопки Clear)
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
        
        // Удаляем предыдущий результат поиска
        this.removeSearchResult();
        
        const resultElement = document.createElement('div');
        resultElement.className = 'search-result';
        resultElement.id = 'search-result-item';
        
        resultElement.innerHTML = `
            <div style="display: flex; justify-content: space-between; align-items: start;">
                <div>
                    <h4>${this.escapeHtml(chatData.chat_name)}</h4>
                    <p>ID: ${chatData.chat_id} • Members: ${chatData.member_count}</p>
                </div>
                <button onclick="chatApp.closeSearchResult()" style="background: transparent; border: none; color: #999; cursor: pointer; font-size: 18px;">×</button>
            </div>
            <button onclick="chatApp.joinChat(${chatData.chat_id})" style="margin-top: 8px;">Join Chat</button>
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

    // Метод для закрытия результата поиска (очищает поле поиска и удаляет результат)
    closeSearchResult() {
        this.removeSearchResult();
        
        // Очищаем поле поиска
        const searchInput = document.getElementById('chat-search');
        if (searchInput) {
            searchInput.value = '';
        }
    }
    
    // Метод для удаления результата поиска
    removeSearchResult() {
        const existingResult = document.getElementById('search-result-item');
        if (existingResult) {
            existingResult.remove();
        }
    }

    // UI Rendering
    renderChats() {
        const chatList = document.getElementById('chat-list');
        if (!chatList) {
            console.error('chat-list element not found!');
            return;
        }
        
        // Удаляем только обычные чаты, оставляя результат поиска
        const itemsToRemove = [];
        for (const child of chatList.children) {
            if (!child.id || child.id !== 'search-result-item') {
                itemsToRemove.push(child);
            }
        }
        
        itemsToRemove.forEach(item => item.remove());
        
        if (this.chats.length === 0 && !document.getElementById('search-result-item')) {
            chatList.innerHTML = '<div class="no-chats" style="color: #999; padding: 10px;">No chats yet. Create one or search for existing chats!</div>';
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
        if (!messagesContainer) return;
        
        messagesContainer.innerHTML = '';
        
        if (!messages || messages.length === 0) {
            messagesContainer.innerHTML = `
                <div style="text-align: center; padding: 2rem; color: #999;">
                    <p>No messages yet. Start the conversation!</p>
                </div>
            `;
            return;
        }
        
        messages.forEach(msg => {
            const messageElement = document.createElement('div');
            messageElement.className = `message-item ${msg.sender_id == this.currentUserId ? 'own' : 'other'}`;
            messageElement.innerHTML = `
                <div class="message-sender">${msg.sender_name}</div>
                <div class="message-content">${this.escapeHtml(msg.content)}</div>
                <div class="message-time">${msg.timestamp}</div>
            `;
            messagesContainer.appendChild(messageElement);
        });
        
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
        
        const currentUserElement = document.getElementById('current-user');
        const currentUserIdElement = document.getElementById('current-user-id');
        
        if (currentUserElement) {
            currentUserElement.textContent = this.currentUser || '';
        }
        
        if (currentUserIdElement) {
            currentUserIdElement.textContent = this.currentUserId || '';
        }
        
        this.currentChat = null;
        this.clearChatInterface();
        
        this.loadChats();
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
        const messageInput = document.getElementById('message-input');
        const sendButton = document.getElementById('send-button');
        
        if (messagesContainer) messagesContainer.innerHTML = '';
        if (currentChatName) currentChatName.textContent = 'Select a Chat';
        if (messageInput) {
            messageInput.disabled = true;
            messageInput.value = '';
            messageInput.placeholder = 'Select a chat to start messaging...';
        }
        if (sendButton) sendButton.disabled = true;
        
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
        }, 2000);
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
    
    if (chatName.trim()) {
        chatApp.createChat(chatName);
    }
}

function hideCreateChat() {
    chatApp.hideCreateChat();
}

function searchChat() {
    chatApp.searchChat();
}


// Enter key handlers
document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('password').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') login();
    });
    
    document.getElementById('reg-password').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') register();
    });
    
    document.getElementById('message-input').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') sendMessage();
    });
    
    document.getElementById('new-chat-name').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') createChat();
    });
    
    document.getElementById('chat-search').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') searchChat();
    });
});