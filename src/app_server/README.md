# App Server

## Installation

### Creates your own https key and cert files

1. Change working directory to where you installed whole project, for example:
   ```
   $ cd installed
   ```

2. Create directory for storing the needed files:
   ```
   $ mkdir -p share/app_server && cd share/app_server
   ```

3. Creates the files:
   ```
   $ openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365
   ```
   Then you need to type some information, just follow the prompt hint.
