
Installation

Ubuntu 16.04

sudo apt-get install php-bcmath
sudo apt-get install php-mbstring

sudo service apache2 restart

cd /var/www/sites/all/libraries
mkdir neo4j-php-client
cd neo4j-php-client
composer require graphaware/neo4j-php-client:^4.0
