#!/usr/bin/env python3
# **************************************************************************** #
#                                                                              #
#    env.py - Environment Variables CGI Script                                 #
#                                                                              #
#    By: fcela-ga <fcela-ga@student.42barcelona.com>                           #
#                                                                              #
#    Plain text output for easy parsing in CI/CD pipelines                     #
#                                                                              #
# **************************************************************************** #

import os
import sys

# Output headers - text/plain for easy CI parsing
print("Content-Type: text/plain; charset=utf-8")
print("")

# Required CGI environment variables per RFC 3875
REQUIRED_VARS = [
    'REQUEST_METHOD',
    'QUERY_STRING',
    'CONTENT_TYPE',
    'CONTENT_LENGTH',
    'SERVER_NAME',
    'SERVER_PORT',
    'SERVER_PROTOCOL',
    'GATEWAY_INTERFACE',
    'SCRIPT_NAME',
    'PATH_INFO',
    'PATH_TRANSLATED',
    'REMOTE_ADDR',
    'REMOTE_HOST',
    'HTTP_HOST',
    'HTTP_USER_AGENT',
    'HTTP_ACCEPT',
    'HTTP_COOKIE'
]

print("=" * 60)
print("CGI Environment Check")
print("=" * 60)
print("")

set_count = 0
not_set_count = 0

print("Required CGI Variables (RFC 3875):")
print("-" * 40)

for var in REQUIRED_VARS:
    value = os.environ.get(var)
    if value is not None:
        status = "OK"
        set_count += 1
        # Truncate long values for readability
        display_value = value if len(value) <= 50 else value[:47] + "..."
    else:
        status = "NOT_SET"
        not_set_count += 1
        display_value = "(not set)"
    
    print(f"  [{status:7}] {var}: {display_value}")

print("")
print(f"Summary: {set_count}/{len(REQUIRED_VARS)} required variables set")
print("")

# HTTP Headers section
print("=" * 60)
print("HTTP Headers (from environment)")
print("=" * 60)
print("")

http_headers = [(k, v) for k, v in sorted(os.environ.items()) if k.startswith('HTTP_')]
if http_headers:
    for key, value in http_headers:
        display_value = value if len(value) <= 50 else value[:47] + "..."
        print(f"  {key}: {display_value}")
else:
    print("  (no HTTP headers found)")

print("")

# All environment variables
print("=" * 60)
print("All Environment Variables")
print("=" * 60)
print("")

for key in sorted(os.environ.keys()):
    value = os.environ[key]
    display_value = value if len(value) <= 60 else value[:57] + "..."
    print(f"  {key}={display_value}")

print("")
print("=" * 60)
print(f"Total variables: {len(os.environ)}")
print("=" * 60)
