# Goliat
Goliat is a project for tracking social media which started as a personal project around 2014.

# How does it work?
The Goliat Collector is a PHP script which can be executed as a daemon.
Using a plugin, it connects to Facebook and Twitter periodically to get data and store it into the local machine.

The data stored will depend on the definition of the 'sondas'.
Each sonda will define:
- Plugin used to connect to the data source (Twitter Search, Twitter User Feed, Facebook Page...)
- Search Query or Account ID.

# Get Started
To run Goliat in your local machine you will need to follow these steps:
- Download the PHP source code from here, and extract into a directory in your local machine (you can also clone the git repository)
- Edit config.inc to include your Access Tokens for Facebook and Twitter (Learn more about how to create tokens https://developers.facebook.com/docs/facebook-login/access-tokens https://developer.twitter.com/en/docs/basics/authentication/guides/access-tokens)
- Make sure you have the right permissions on the paths specified in config.inc.
- Save your config.inc file.
- Configure the sources you want to track in the social media in \_path_sondas directory. You can use the samples provided in /sondas_examples to create your own.
- Execute Goliat Collector running: <b>php goliat-collector.php</b>
- If everything goes right, you should see something like this in the logfile <i>/var/log/goliat_collector.log</i>:
<pre>++++++++ GOLIAT COLLECTOR STARTED (15/03/2018 20:10:39) ++++++++
Executing plugin -> Facebookuser(trumpOnFacebook)
25/25 comments
Executing plugin -> Twitter(whiteHouseTwitterMention)
25/100 comments
Executing plugin -> Twitteruser(trumpOnTwitter)
20/20 comments
.......</pre>
- You should see how a bunch of new XML files are being created into your \_path_data directory.

# Data
Each post or tweet is stored into a single XML file. This is an example of a post collected by Goliat 'facebookuser' plugin:

<i>/data/dump/<b>20180227214109.trumpOnFacebook.facebookuser.8f5fb7986bd40a787c004f9ecaeaf921.xml</b></i>:
<pre>
&lt;?xml version="1.0"?&gt;
&lt;post&gt;&lt;from_user&gt;Donald J. Trump&lt;/from_user&gt;&lt;from_user_id&gt;153080620724&lt;/from_user_id&gt;&lt;profile_image_url&gt;https://graph.facebook.com/153080620724/picture&lt;/profile_image_url&gt;&lt;text&gt;Tremendous things are happening!

"American consumers are the most confident they've been since 2000&#x2026;The unemployment rate has stayed at a 17-year low." https://usat.ly/2Fc6RmK&lt;/text&gt;&lt;created_at&gt;2018-02-27 21:41:09&lt;/created_at&gt;&lt;type&gt;photo&lt;/type&gt;&lt;link&gt;https://www.facebook.com/DonaldTrump/photos/a.488852220724.393301.153080620724/10160655279815725/?type=3&lt;/link&gt;&lt;id&gt;153080620724_10160655287330725&lt;/id&gt;&lt;/post&gt;
</pre>
