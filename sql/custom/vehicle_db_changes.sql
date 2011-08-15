ALTER TABLE creature_template
  ADD COLUMN `PowerType` tinyint(3) unsigned NOT NULL default '0' AFTER `MaxHealth`,
  ADD COLUMN `spell5` mediumint(8) unsigned NOT NULL default '0' AFTER `spell4`,
  ADD COLUMN `spell6` mediumint(8) unsigned NOT NULL default '0' AFTER `spell5`,
  ADD COLUMN `spell7` mediumint(8) unsigned NOT NULL default '0' AFTER `spell6`,
  ADD COLUMN `spell8` mediumint(8) unsigned NOT NULL default '0' AFTER `spell7`;

DROP TABLE IF EXISTS `vehicle_accessory`;
CREATE TABLE `vehicle_accessory` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `accessory_entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `seat_id` tinyint(1) NOT NULL DEFAULT '0',
  `minion` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `description` text NOT NULL,
  PRIMARY KEY (`entry`,`seat_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Vehicle Accessory System';
