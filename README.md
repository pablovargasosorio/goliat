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
Each post or tweet is stored into a single XML file. This is an example of a post collected by Goliat 'facebookuser' plugin:

<pre>
&lt;?xml version="1.0"?&gt;
&lt;post&gt;&lt;from_user&gt;Donald J. Trump&lt;/from_user&gt;&lt;from_user_id&gt;153080620724&lt;/from_user_id&gt;&lt;profile_image_url&gt;https://graph.facebook.com/153080620724/picture&lt;/profile_image_url&gt;&lt;text&gt;Tremendous things are happening!

"American consumers are the most confident they've been since 2000&#x2026;The unemployment rate has stayed at a 17-year low." https://usat.ly/2Fc6RmK&lt;/text&gt;&lt;created_at&gt;2018-02-27 21:41:09&lt;/created_at&gt;&lt;type&gt;photo&lt;/type&gt;&lt;link&gt;https://www.facebook.com/DonaldTrump/photos/a.488852220724.393301.153080620724/10160655279815725/?type=3&lt;/link&gt;&lt;id&gt;153080620724_10160655287330725&lt;/id&gt;&lt;/post&gt;
</pre>