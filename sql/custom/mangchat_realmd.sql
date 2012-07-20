DROP TABLE IF EXISTS `mangchat`;
CREATE TABLE `mangchat` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `host` text,
  `port` int(11) NOT NULL DEFAULT '0',
  `user` text,
  `pass` text,
  `nick` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `mangchat_links`;
CREATE TABLE `mangchat_links` (
  `mangchat_id` int(11) unsigned NOT NULL DEFAULT '0',
  `wow_channel` varchar(255) NOT NULL DEFAULT '',
  `wow_channel_options` int(11) unsigned NOT NULL DEFAULT '0',
  `irc_channel` varchar(255) NOT NULL DEFAULT '',
  `irc_channel_options` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`mangchat_id`,`wow_channel`,`irc_channel`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
