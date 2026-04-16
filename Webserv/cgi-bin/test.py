#!/usr/bin/env python3
# **************************************************************************** #
#                                                                              #
#    test.py - CGI Test Script                                                 #
#                                                                              #
#    By: fcela-ga <fcela-ga@student.42barcelona.com>                           #
#                                                                              #
#    Demonstrates CGI functionality with GET and POST methods                  #
#                                                                              #
# **************************************************************************** #

import os
import sys
import cgi
import html

def main():
    # Print CGI header
    print("Content-Type: text/html; charset=utf-8")
    print()  # Empty line to end headers

    # Get request method
    method = os.environ.get('REQUEST_METHOD', 'GET')
    query_string = os.environ.get('QUERY_STRING', '')

    # Parse form data
    form = cgi.FieldStorage()

    # Build HTML response
    print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CGI Test Script - Webserv</title>
    <style>
        * { box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
            max-width: 900px;
            margin: 0 auto;
            padding: 2rem;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
        }
        h1 { color: #2c3e50; margin-top: 0; }
        .card {
            background: white;
            padding: 1.5rem;
            border-radius: 12px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.15);
            margin: 1rem 0;
        }
        .card h2 { color: #3498db; margin-top: 0; border-bottom: 2px solid #eee; padding-bottom: 0.5rem; }
        table { width: 100%; border-collapse: collapse; }
        th, td { padding: 0.6rem; text-align: left; border-bottom: 1px solid #eee; }
        th { color: #7f8c8d; font-weight: 500; width: 35%; }
        td { font-family: 'Monaco', 'Menlo', monospace; font-size: 0.9rem; word-break: break-all; }
        form { margin-top: 1rem; }
        input, textarea, select {
            display: block;
            width: 100%;
            padding: 0.75rem;
            margin: 0.5rem 0;
            border: 2px solid #ddd;
            border-radius: 8px;
            font-size: 1rem;
            transition: border-color 0.3s;
        }
        input:focus, textarea:focus, select:focus { border-color: #3498db; outline: none; }
        button {
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
            border: none;
            padding: 0.75rem 1.5rem;
            border-radius: 8px;
            cursor: pointer;
            font-size: 1rem;
            margin-top: 0.5rem;
            transition: opacity 0.3s;
        }
        button:hover { opacity: 0.9; }
        .success { background: #d4edda; color: #155724; padding: 1rem; border-radius: 8px; margin-bottom: 1rem; }
        .method-badge {
            display: inline-block;
            padding: 0.3rem 0.8rem;
            border-radius: 20px;
            font-weight: bold;
            font-size: 0.85rem;
        }
        .method-get { background: #d4edda; color: #155724; }
        .method-post { background: #cce5ff; color: #004085; }
        a { color: #667eea; text-decoration: none; }
        a:hover { text-decoration: underline; }
        .nav { display: flex; gap: 1rem; flex-wrap: wrap; }
        .nav a { 
            padding: 0.5rem 1rem; 
            background: #f8f9fa; 
            border-radius: 6px;
            transition: background 0.3s;
        }
        .nav a:hover { background: #e9ecef; text-decoration: none; }
    </style>
</head>
<body>
    <div class="card">
        <h1>🐍 Python CGI Test Script</h1>
        <p>This script demonstrates CGI functionality with the Webserv HTTP server.</p>
    </div>

    <div class="card">
        <h2>Request Information</h2>
        <table>""")

    print(f'<tr><th>Method:</th><td><span class="method-badge method-{method.lower()}">{html.escape(method)}</span></td></tr>')
    print(f'<tr><th>Query String:</th><td>{html.escape(query_string or "(empty)")}</td></tr>')
    print(f'<tr><th>Content-Type:</th><td>{html.escape(os.environ.get("CONTENT_TYPE", "(not set)"))}</td></tr>')
    print(f'<tr><th>Content-Length:</th><td>{html.escape(os.environ.get("CONTENT_LENGTH", "(not set)"))}</td></tr>')

    print("""
        </table>
    </div>

    <div class="card">
        <h2>Server Information</h2>
        <table>""")

    server_vars = [
        ('SERVER_NAME', 'Server Name'),
        ('SERVER_PORT', 'Server Port'),
        ('SERVER_PROTOCOL', 'Protocol'),
        ('SERVER_SOFTWARE', 'Server Software'),
        ('GATEWAY_INTERFACE', 'CGI Version'),
        ('SCRIPT_NAME', 'Script Name'),
        ('PATH_INFO', 'Path Info'),
        ('REMOTE_ADDR', 'Remote Address'),
    ]

    for var, label in server_vars:
        value = os.environ.get(var, '(not set)')
        print(f'<tr><th>{label}:</th><td>{html.escape(value)}</td></tr>')

    print("""
        </table>
    </div>""")

    # Show form data if POST
    if method == 'POST' and len(form) > 0:
        print("""
    <div class="card">
        <h2 class="success">✅ Form Data Received (POST)</h2>
        <table>""")
        for key in form.keys():
            value = form.getvalue(key)
            if isinstance(value, list):
                value = ', '.join(str(v) for v in value)
            print(f'<tr><th>{html.escape(key)}:</th><td>{html.escape(str(value))}</td></tr>')
        print("""
        </table>
    </div>""")

    # Show query parameters if GET with params
    if method == 'GET' and query_string:
        print("""
    <div class="card">
        <h2>Query Parameters (GET)</h2>
        <table>""")
        for key in form.keys():
            value = form.getvalue(key)
            if isinstance(value, list):
                value = ', '.join(str(v) for v in value)
            print(f'<tr><th>{html.escape(key)}:</th><td>{html.escape(str(value))}</td></tr>')
        print("""
        </table>
    </div>""")

    # HTTP Headers
    print("""
    <div class="card">
        <h2>HTTP Client Headers</h2>
        <table>""")

    http_vars = [(k, v) for k, v in sorted(os.environ.items()) if k.startswith('HTTP_')]
    if http_vars:
        for key, value in http_vars:
            header_name = key[5:].replace('_', '-').title()
            print(f'<tr><th>{header_name}:</th><td>{html.escape(value)}</td></tr>')
    else:
        print('<tr><td colspan="2">(no HTTP headers)</td></tr>')

    print("""
        </table>
    </div>""")

    # Test forms
    print("""
    <div class="card">
        <h2>Test POST Form</h2>
        <form method="POST" action="/cgi-bin/test.py">
            <input type="text" name="name" placeholder="Your name" required>
            <input type="email" name="email" placeholder="Your email">
            <textarea name="message" placeholder="Your message" rows="3"></textarea>
            <button type="submit">Submit POST Request</button>
        </form>
    </div>

    <div class="card">
        <h2>Test GET Form</h2>
        <form method="GET" action="/cgi-bin/test.py">
            <input type="text" name="search" placeholder="Search query">
            <select name="filter">
                <option value="">Select filter...</option>
                <option value="recent">Recent</option>
                <option value="popular">Popular</option>
                <option value="alphabetical">Alphabetical</option>
            </select>
            <button type="submit">Submit GET Request</button>
        </form>
    </div>

    <div class="card">
        <h2>Navigation</h2>
        <div class="nav">
            <a href="/">🏠 Home</a>
            <a href="/cgi-bin/info.py">ℹ️ System Info</a>
            <a href="/cgi-bin/env.py">🔧 Environment</a>
            <a href="/cgi-bin/session.py">🍪 Session Demo</a>
            <a href="/cgi-bin/upload.py">📤 File Upload</a>
        </div>
    </div>
</body>
</html>""")

if __name__ == '__main__':
    main()
