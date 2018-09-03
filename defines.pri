DEFINES += ROBOTIC_SERVER_APP_VERSION=\\\"3.0.0\\\"

DEFINES += GIT_TIMESTAMP=\\\"$$system($$quote(git log -n 1 --format=format:\\\"%ai\\\"))\\\"

DEFINES += GIT_BRANCH=\\\"$$system($$quote(git rev-parse --abbrev-ref HEAD))\\\"
