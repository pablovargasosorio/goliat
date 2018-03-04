# Goliat
Goliat is a project for tracking social media which started as a personal project around 2014.
After a while, I decided to make some part of my code public. And maybe, make it a collaborative Open Source project.

# How does it work?
The Goliat Collector is a PHP script which can be executed as a daemon.
Using a plugin, it connects to Facebook and Twitter periodically to get data and store it into the local machine.

The data stored will depend on the definition of the 'sondas'.
Each sonda will define:
- Plugin used to connect to the data source (Twitter Search, Twitter User Feed, Facebook Page...)
- Search Query or Account ID.

# Data
Each post or tweet is stored into a single XML file. This is an example of a tweet collected by Goliat 'twitter' plugin:

<pre>
<?xml version="1.0"?>
<post><created_at>2018-03-03 18:55:40</created_at><id_str>969994732540743680</id_str><text>Los hospitales valencianos privatizados vuelven al control p&#xFA;blico pese a la campa&#xF1;a legal en contra https://t.co/sQfBGmA3aH</text><source>&lt;a href="http://twitter.com" rel="nofollow"&gt;Twitter Web Client&lt;/a&gt;</source><retweet_count>739</retweet_count><favorite_count>968</favorite_count><lang>es</lang><from_user>iescolar</from_user><from_user_name>Ignacio Escolar</from_user_name><from_user_id>14436317</from_user_id><profile_image_url>http://pbs.twimg.com/profile_images/863361553025949696/KXqNkYUA_normal.jpg</profile_image_url></post>
</pre>