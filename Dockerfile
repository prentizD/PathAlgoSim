FROM python:3.12-alpine

RUN mkdir /server

ADD ./server /server

WORKDIR /server

EXPOSE 8080

CMD ["python", "-m" , "http.server", "8080"]