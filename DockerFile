FROM ubuntu:18.04

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y \
    build-essential \
    qtbase5-dev \
    qt5-qmake \
    qt5-default

COPY . /app

WORKDIR /app/server_1.3
RUN qmake echoServer.pro
RUN make 
EXPOSE 33333

CMD ["./echoServer"]
