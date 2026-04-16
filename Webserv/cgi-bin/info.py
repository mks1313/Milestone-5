#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
CGI System Info Script
Displays system and environment information
"""

import os
import sys
import platform

def main():
    print("Content-Type: text/html; charset=utf-8")
    print()
    
    print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>System Information - CGI</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            max-width: 900px;
            margin: 0 auto;
            padding: 2rem;
            background: #f5f5f5;
        }
        h1 { color: #2c3e50; border-bottom: 3px solid #3498db; padding-bottom: 10px; }
        h2 { color: #3498db; margin-top: 2rem; }
        .card {
            background: white;
            padding: 1.5rem;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            margin: 1rem 0;
        }
        table { width: 100%; border-collapse: collapse; }
        th, td { padding: 10px; text-align: left; border-bottom: 1px solid #eee; }
        th { background: #f8f8f8; color: #666; width: 30%; }
        td { font-family: monospace; word-break: break-all; }
        .success { color: #27ae60; }
        a { color: #3498db; }
    </style>
</head>
<body>
    <h1>System Information</h1>
    
    <div class="card">
        <h2>Python Environment</h2>
        <table>""")
    
    print(f"<tr><th>Python Version:</th><td>{sys.version}</td></tr>")
    print(f"<tr><th>Platform:</th><td>{platform.platform()}</td></tr>")
    print(f"<tr><th>System:</th><td>{platform.system()}</td></tr>")
    print(f"<tr><th>Architecture:</th><td>{platform.machine()}</td></tr>")
    print(f"<tr><th>Executable:</th><td>{sys.executable}</td></tr>")
    
    print("""
        </table>
    </div>
    
    <div class="card">
        <h2>CGI Environment Variables</h2>
        <table>""")
    
    cgi_vars = [
        'REQUEST_METHOD', 'QUERY_STRING', 'CONTENT_TYPE', 'CONTENT_LENGTH',
        'SCRIPT_NAME', 'SCRIPT_FILENAME', 'PATH_INFO', 'PATH_TRANSLATED',
        'REQUEST_URI', 'DOCUMENT_ROOT', 'SERVER_SOFTWARE', 'SERVER_NAME',
        'SERVER_PORT', 'SERVER_PROTOCOL', 'GATEWAY_INTERFACE',
        'REMOTE_ADDR', 'REMOTE_PORT', 'REMOTE_HOST',
        'HTTP_HOST', 'HTTP_USER_AGENT', 'HTTP_ACCEPT', 'HTTP_ACCEPT_LANGUAGE',
        'HTTP_ACCEPT_ENCODING', 'HTTP_CONNECTION', 'HTTP_COOKIE', 'HTTP_REFERER'
    ]
    
    for var in cgi_vars:
        value = os.environ.get(var, '<not set>')
        print(f"<tr><th>{var}:</th><td>{value}</td></tr>")
    
    print("""
        </table>
    </div>
    
    <div class="card">
        <h2>All Environment Variables</h2>
        <table>""")
    
    for key in sorted(os.environ.keys()):
        value = os.environ[key]
        # Escape HTML
        value = value.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')
        print(f"<tr><th>{key}:</th><td>{value}</td></tr>")
    
    print("""
        </table>
    </div>
    
    <div class="card">
        <p><a href="/">Back to Home</a> | <a href="/cgi-bin/test.py">Test CGI</a></p>
    </div>
</body>
</html>""")

if __name__ == '__main__':
    main()
