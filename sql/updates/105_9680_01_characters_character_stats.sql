ALTER TABLE character_db_version CHANGE COLUMN required_102_9687_01_characters_character_queststatus_daily required_105_9680_01_characters_character_stats bit;

DROP TABLE IF EXISTS `character_stats`;
CREATE TABLE `character_stats` (
  `guid` int(11) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier, Low part',
  `maxhealth` int(10) UNSIGNED NOT NULL default '0',
  `maxpower1` int(10) UNSIGNED NOT NULL default '0',
  `maxpower2` int(10) UNSIGNED NOT NULL default '0',
  `maxpower3` int(10) UNSIGNED NOT NULL default '0',
  `maxpower4` int(10) UNSIGNED NOT NULL default '0',
  `maxpower5` int(10) UNSIGNED NOT NULL default '0',
  `maxpower6` int(10) UNSIGNED NOT NULL default '0',
  `maxpower7` int(10) UNSIGNED NOT NULL default '0',
  `strength` int(10) UNSIGNED NOT NULL default '0',
  `agility` int(10) UNSIGNED NOT NULL default '0',
  `stamina` int(10) UNSIGNED NOT NULL default '0',
  `intellect` int(10) UNSIGNED NOT NULL default '0',
  `spirit` int(10) UNSIGNED NOT NULL default '0',
  `armor` int(10) UNSIGNED NOT NULL default '0',
  `resHoly` int(10) UNSIGNED NOT NULL default '0',
  `resFire` int(10) UNSIGNED NOT NULL default '0',
  `resNature` int(10) UNSIGNED NOT NULL default '0',
  `resFrost` int(10) UNSIGNED NOT NULL default '0',
  `resShadow` int(10) UNSIGNED NOT NULL default '0',
  `resArcane` int(10) UNSIGNED NOT NULL default '0',
  `blockPct` float UNSIGNED NOT NULL default '0',
  `dodgePct` float UNSIGNED NOT NULL default '0',
  `parryPct` float UNSIGNED NOT NULL default '0',
  `critPct` float UNSIGNED NOT NULL default '0',  
  `rangedCritPct` float UNSIGNED NOT NULL default '0',
  `spellCritPct` float UNSIGNED NOT NULL default '0',
  `attackPower` int(10) UNSIGNED NOT NULL default '0',
  `rangedAttackPower` int(10) UNSIGNED NOT NULL default '0',
  `spellPower` int(10) UNSIGNED NOT NULL default '0',
   
  PRIMARY KEY  (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;