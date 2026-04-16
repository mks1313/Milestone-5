#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
CGI File Upload Handler
Handles file uploads via CGI
"""

import os
import sys
import cgi
import html
import hashlib
import time

UPLOAD_DIR = '/tmp/webserv_uploads'

def ensure_upload_dir():
    """Create upload directory if it doesn't exist"""
    if not os.path.exists(UPLOAD_DIR):
        try:
            os.makedirs(UPLOAD_DIR)
        except:
            pass

def get_safe_filename(filename):
    """Sanitize filename for safe storage"""
    # Remove path components
    filename = os.path.basename(filename)
    # Remove potentially dangerous characters
    safe_chars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_'
    result = ''.join(c if c in safe_chars else '_' for c in filename)
    # Add timestamp to avoid collisions
    timestamp = str(int(time.time()))
    name, ext = os.path.splitext(result)
    return f"{name}_{timestamp}{ext}"

def format_size(size):
    """Format file size in human readable format"""
    for unit in ['B', 'KB', 'MB', 'GB']:
        if size < 1024:
            return f"{size:.1f} {unit}"
        size /= 1024
    return f"{size:.1f} TB"

def main():
    ensure_upload_dir()
    
    method = os.environ.get('REQUEST_METHOD', 'GET')
    uploaded_files = []
    error_message = ""
    
    if method == 'POST':
        content_type = os.environ.get('CONTENT_TYPE', '')
        
        if 'multipart/form-data' in content_type:
            try:
                form = cgi.FieldStorage()
                
                # Handle 'file' field (single or multiple)
                if 'file' in form:
                    file_item = form['file']
                    
                    # Handle single file or list of files
                    if isinstance(file_item, list):
                        files = file_item
                    else:
                        files = [file_item]
                    
                    for item in files:
                        if item.filename:
                            safe_name = get_safe_filename(item.filename)
                            filepath = os.path.join(UPLOAD_DIR, safe_name)
                            
                            # Read and save file
                            data = item.file.read()
                            with open(filepath, 'wb') as f:
                                f.write(data)
                            
                            uploaded_files.append({
                                'original_name': item.filename,
                                'saved_name': safe_name,
                                'size': len(data),
                                'path': filepath
                            })
                
                if not uploaded_files:
                    error_message = "No file was uploaded. Please select a file."
                    
            except Exception as e:
                error_message = f"Error processing upload: {str(e)}"
        else:
            error_message = f"Invalid content type: {content_type}. Expected multipart/form-data."
    
    # List existing uploads
    existing_files = []
    try:
        for filename in os.listdir(UPLOAD_DIR):
            filepath = os.path.join(UPLOAD_DIR, filename)
            if os.path.isfile(filepath):
                stat = os.stat(filepath)
                existing_files.append({
                    'name': filename,
                    'size': stat.st_size,
                    'modified': time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(stat.st_mtime))
                })
    except:
        pass
    
    # Output response
    print("Content-Type: text/html; charset=utf-8")
    print()
    
    print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>File Upload - CGI</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            max-width: 900px;
            margin: 0 auto;
            padding: 2rem;
            background: #f5f5f5;
        }
        h1 { color: #2c3e50; border-bottom: 3px solid #27ae60; padding-bottom: 10px; }
        h2 { color: #27ae60; margin-top: 2rem; }
        .card {
            background: white;
            padding: 1.5rem;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            margin: 1rem 0;
        }
        .success {
            background: #d4edda;
            color: #155724;
            padding: 1rem;
            border-radius: 8px;
            margin-bottom: 1rem;
        }
        .error {
            background: #f8d7da;
            color: #721c24;
            padding: 1rem;
            border-radius: 8px;
            margin-bottom: 1rem;
        }
        table { width: 100%; border-collapse: collapse; }
        th, td { padding: 10px; text-align: left; border-bottom: 1px solid #eee; }
        th { background: #f8f8f8; color: #666; }
        form { margin: 1rem 0; }
        input[type="file"] {
            padding: 1rem;
            border: 2px dashed #ddd;
            border-radius: 8px;
            width: 100%;
            margin-bottom: 1rem;
            cursor: pointer;
        }
        input[type="file"]:hover { border-color: #27ae60; }
        button {
            padding: 0.75rem 1.5rem;
            background: linear-gradient(135deg, #27ae60, #219a52);
            color: white;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            font-size: 1rem;
        }
        button:hover { opacity: 0.9; }
        a { color: #27ae60; }
        .file-info { font-family: monospace; font-size: 0.9rem; }
    </style>
</head>
<body>
    <h1>File Upload Handler (CGI)</h1>""")
    
    if error_message:
        print(f'<div class="error">{html.escape(error_message)}</div>')
    
    if uploaded_files:
        print('<div class="success"><strong>Files uploaded successfully!</strong><ul>')
        for f in uploaded_files:
            print(f'<li>{html.escape(f["original_name"])} - {format_size(f["size"])} - Saved as: {html.escape(f["saved_name"])}</li>')
        print('</ul></div>')
    
    print("""
    <div class="card">
        <h2>Upload New File</h2>
        <form method="POST" action="/cgi-bin/upload.py" enctype="multipart/form-data">
            <input type="file" name="file" required>
            <button type="submit">Upload File</button>
        </form>
        <p><small>Files are saved to the server's temporary directory.</small></p>
    </div>
    
    <div class="card">
        <h2>Previously Uploaded Files</h2>""")
    
    if existing_files:
        print('<table>')
        print('<tr><th>Filename</th><th>Size</th><th>Modified</th></tr>')
        for f in sorted(existing_files, key=lambda x: x['name']):
            print(f'<tr><td class="file-info">{html.escape(f["name"])}</td><td>{format_size(f["size"])}</td><td>{f["modified"]}</td></tr>')
        print('</table>')
    else:
        print('<p>No files uploaded yet.</p>')
    
    print("""
    </div>
    
    <div class="card">
        <h2>Environment Info</h2>
        <table>""")
    
    print(f'<tr><th>REQUEST_METHOD</th><td>{html.escape(os.environ.get("REQUEST_METHOD", "(not set)"))}</td></tr>')
    print(f'<tr><th>CONTENT_TYPE</th><td>{html.escape(os.environ.get("CONTENT_TYPE", "(not set)"))}</td></tr>')
    print(f'<tr><th>CONTENT_LENGTH</th><td>{html.escape(os.environ.get("CONTENT_LENGTH", "(not set)"))}</td></tr>')
    print(f'<tr><th>Upload Directory</th><td>{html.escape(UPLOAD_DIR)}</td></tr>')
    
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
