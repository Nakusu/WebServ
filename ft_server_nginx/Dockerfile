FROM debian:buster
RUN apt-get update && apt-get upgrade && apt-get install nginx wget mariadb-server mariadb-client php7.3-fpm -y
COPY srcs/ var/www/html/
RUN mkdir mkdir /etc/nginx/ssl
RUN mkdir /etc/nginx/ssl/localhost/
RUN mv var/www/html/*.pem /etc/nginx/ssl/localhost/ 