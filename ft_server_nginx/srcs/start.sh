service nginx start
service php7.3-fpm start
service mysql start
mysql -u root -e "CREATE DATABASE wordpress"
mysql -u root -e "CREATE USER 'user'@'localhost' IDENTIFIED BY 'mdp'"
mysql -u root -e "GRANT ALL ON wordpress.* TO 'user'@'localhost' IDENTIFIED BY 'mdp' WITH GRANT OPTION"
mysql -u root -e "FLUSH PRIVILEGES"
service nginx restart
