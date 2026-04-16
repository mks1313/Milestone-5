#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
CGI Session Demo Script
Demonstrates cookie and session management (BONUS)
"""

import os
import sys
import cgi
import html
import hashlib
import time
import random
import string

# Simple in-memory session storage (for demo purposes)
SESSION_FILE = '/tmp/webserv_sessions.txt'

def generate_session_id():
    """Generate a random session ID"""
    chars = string.ascii_letters + string.digits
    return ''.join(random.choice(chars) for _ in range(32))

def get_cookie(name):
    """Get cookie value from HTTP_COOKIE"""
    cookies = os.environ.get('HTTP_COOKIE', '')
    for cookie in cookies.split(';'):
        cookie = cookie.strip()
        if '=' in cookie:
            key, value = cookie.split('=', 1)
            if key.strip() == name:
                return value.strip()
    return None

def load_sessions():
    """Load sessions from file"""
    sessions = {}
    try:
        with open(SESSION_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 3:
                    sid, timestamp, data = parts[0], parts[1], '|'.join(parts[2:])
                    sessions[sid] = {'timestamp': float(timestamp), 'data': data}
    except:
        pass
    return sessions

def save_sessions(sessions):
    """Save sessions to file"""
    try:
        with open(SESSION_FILE, 'w') as f:
            for sid, info in sessions.items():
                f.write(f"{sid}|{info['timestamp']}|{info['data']}\n")
    except:
        pass

def main():
    form = cgi.FieldStorage()
    action = form.getvalue('action', '')
    
    # Get or create session
    session_id = get_cookie('WEBSERV_SESSION')
    sessions = load_sessions()
    
    is_new_session = False
    session_data = {}
    
    if session_id and session_id in sessions:
        # Existing session
        session_info = sessions[session_id]
        try:
            # Parse simple key=value pairs
            for pair in session_info['data'].split('&'):
                if '=' in pair:
                    k, v = pair.split('=', 1)
                    session_data[k] = v
        except:
            pass
    else:
        # New session
        session_id = generate_session_id()
        is_new_session = True
    
    # Handle actions
    message = ""
    if action == 'set_name':
        name = form.getvalue('name', 'Guest')
        session_data['name'] = html.escape(name)
        message = f"Name set to: {session_data['name']}"
    elif action == 'increment':
        count = int(session_data.get('count', 0)) + 1
        session_data['count'] = str(count)
        message = f"Counter incremented to: {count}"
    elif action == 'reset':
        session_data = {}
        message = "Session data cleared!"
    elif action == 'destroy':
        if session_id in sessions:
            del sessions[session_id]
            save_sessions(sessions)
        print("Content-Type: text/html; charset=utf-8")
        print("Set-Cookie: WEBSERV_SESSION=deleted; Path=/; Max-Age=0")
        print()
        print("""<!DOCTYPE html><html><head><meta http-equiv="refresh" content="2;url=/cgi-bin/session.py">
        <title>Session Destroyed</title></head><body>
        <h1>Session Destroyed</h1><p>Redirecting...</p></body></html>""")
        return
    
    # Save session
    data_str = '&'.join(f"{k}={v}" for k, v in session_data.items())
    sessions[session_id] = {'timestamp': time.time(), 'data': data_str}
    save_sessions(sessions)
    
    # Output headers
    print("Content-Type: text/html; charset=utf-8")
    if is_new_session:
        print(f"Set-Cookie: WEBSERV_SESSION={session_id}; Path=/; HttpOnly")
    print()
    
    # Output HTML
    name = session_data.get('name', 'Guest')
    count = session_data.get('count', '0')
    
    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Session Demo - Webserv</title>
    <style>
        body {{
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            max-width: 800px;
            margin: 0 auto;
            padding: 2rem;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
        }}
        .card {{
            background: white;
            padding: 2rem;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            margin-bottom: 1.5rem;
        }}
        h1 {{ color: #2c3e50; margin-top: 0; }}
        h2 {{ color: #3498db; }}
        .info {{ background: #e8f4fc; padding: 1rem; border-radius: 8px; margin: 1rem 0; }}
        .success {{ background: #d4edda; color: #155724; padding: 1rem; border-radius: 8px; }}
        .session-id {{ font-family: monospace; font-size: 0.9rem; color: #666; word-break: break-all; }}
        form {{ margin: 1rem 0; }}
        input[type="text"] {{
            padding: 0.75rem;
            border: 2px solid #ddd;
            border-radius: 8px;
            font-size: 1rem;
            width: 200px;
        }}
        button {{
            padding: 0.75rem 1.5rem;
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            font-size: 1rem;
            margin: 0.25rem;
        }}
        button:hover {{ opacity: 0.9; }}
        .btn-danger {{ background: linear-gradient(135deg, #e74c3c, #c0392b); }}
        .btn-warning {{ background: linear-gradient(135deg, #f39c12, #d68910); }}
        table {{ width: 100%; border-collapse: collapse; margin: 1rem 0; }}
        th, td {{ padding: 10px; text-align: left; border-bottom: 1px solid #eee; }}
        th {{ background: #f8f8f8; }}
        a {{ color: #667eea; }}
    </style>
</head>
<body>
    <div class="card">
        <h1>Session Demo (Bonus Feature)</h1>
        <p>This demonstrates cookie-based session management.</p>
        
        {"<div class='success'>" + message + "</div>" if message else ""}
        
        <div class="info">
            <strong>Session ID:</strong><br>
            <span class="session-id">{session_id}</span>
            {"<br><em>(New session created)</em>" if is_new_session else ""}
        </div>
    </div>
    
    <div class="card">
        <h2>Current Session Data</h2>
        <table>
            <tr><th>Name</th><td>{name}</td></tr>
            <tr><th>Counter</th><td>{count}</td></tr>
            <tr><th>Session Age</th><td>{time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(sessions.get(session_id, {}).get('timestamp', time.time())))}</td></tr>
        </table>
    </div>
    
    <div class="card">
        <h2>Test Session Operations</h2>
        
        <form method="POST" action="/cgi-bin/session.py">
            <input type="hidden" name="action" value="set_name">
            <input type="text" name="name" placeholder="Enter your name" required>
            <button type="submit">Set Name</button>
        </form>
        
        <form method="POST" action="/cgi-bin/session.py" style="display:inline;">
            <input type="hidden" name="action" value="increment">
            <button type="submit">Increment Counter</button>
        </form>
        
        <form method="POST" action="/cgi-bin/session.py" style="display:inline;">
            <input type="hidden" name="action" value="reset">
            <button type="submit" class="btn-warning">Clear Data</button>
        </form>
        
        <form method="POST" action="/cgi-bin/session.py" style="display:inline;">
            <input type="hidden" name="action" value="destroy">
            <button type="submit" class="btn-danger">Destroy Session</button>
        </form>
    </div>
    
    <div class="card">
        <h2>Cookie Information</h2>
        <table>
            <tr><th>HTTP_COOKIE</th><td>{html.escape(os.environ.get('HTTP_COOKIE', '(none)'))}</td></tr>
        </table>
        <p><a href="/">Back to Home</a> | <a href="/cgi-bin/test.py">Test CGI</a></p>
    </div>
</body>
</html>""")

if __name__ == '__main__':
    main()
