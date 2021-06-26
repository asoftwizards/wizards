--drop table EStatus;


CREATE TABLE IF NOT EXISTS EStatus (
	StatusID INT NOT NULL AUTO_INCREMENT COMMENT 'Код статуса',
	Name VARCHAR(40) NOT NULL COMMENT 'Статус',
	UserArc BIGINT COMMENT 'Modified by',
	DateArc DATETIME COMMENT 'Modification time',
	PRIMARY KEY (StatusID)
) ENGINE=Innobase DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='Статус';

CREATE TABLE IF NOT EXISTS EStatusArc  (
	StatusID INT COMMENT 'Код статуса',
	Name VARCHAR(40) COMMENT 'Статус',
	UserArc BIGINT COMMENT 'Modified by',
	DateArc DATETIME COMMENT 'Modification time',
	ArcType VARCHAR(1) COMMENT 'ArcType',
	PRIMARY KEY (StatusID, UserArc, DateArc)
) ENGINE=Innobase DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='Table for audit Статус';

INSERT INTO EStatus( StatusID, Name, UserArc, DateArc ) VALUES
( '1', 'На складе', -1, '0000-00-00 00:00:00'  ), 
( '2', 'Транспортируется на базу', -1, '0000-00-00 00:00:00'  ), 
( '3', 'Монтаж', -1, '0000-00-00 00:00:00'  ), 
( '4', 'Используется', -1, '0000-00-00 00:00:00'  ), 
( '5', 'Демонтаж', -1, '0000-00-00 00:00:00'  ), 
( '6', 'Транспортируется на скважину', -1, '0000-00-00 00:00:00'  ), 
( '7', 'На ремонте', -1, '0000-00-00 00:00:00'  );

ALTER TABLE Equipment ADD StatusID INT NOT NULL COMMENT 'Код статуса'; 
ALTER TABLE EquipmentArc ADD StatusID INT NOT NULL COMMENT 'Код статуса'; 

UPDATE Equipment set StatusID = 1 where EStatus='stored';
UPDATE Equipment set StatusID = 2 where EStatus='trans_to_store';
UPDATE Equipment set StatusID = 3 where EStatus='installation';
UPDATE Equipment set StatusID = 4 where EStatus='used';
UPDATE Equipment set StatusID = 5 where EStatus='dismantling';
UPDATE Equipment set StatusID = 6 where EStatus='trans_to_well';
UPDATE Equipment set StatusID = 7 where EStatus='repair';

ALTER TABLE Equipment DROP EStatus;

CREATE INDEX EquipmentEStatus ON Equipment(StatusID);
ALTER TABLE Equipment ADD CONSTRAINT Equipment_EStatus FOREIGN KEY (StatusID) REFERENCES EStatus (StatusID);

