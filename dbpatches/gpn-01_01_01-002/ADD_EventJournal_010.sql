
CREATE TABLE EventJournal (
	EventID INT NOT NULL AUTO_INCREMENT COMMENT 'Код',
	EquipID INT COMMENT 'Код записи',
	PointID INT COMMENT 'Код скважины',
	StatusID INT NOT NULL COMMENT 'Код статуса',
	State VARCHAR(40) NOT NULL COMMENT 'Состояние',
	Note VARCHAR(500) COMMENT 'Обоснование',
	EventDate DATETIME NOT NULL COMMENT 'Дата события',
	TokenID BIGINT NOT NULL COMMENT 'Код пользователя',
	UserArc BIGINT COMMENT 'Modified by',
	DateArc DATETIME COMMENT 'Modification time',
	PRIMARY KEY (EventID)
) ENGINE=Innobase DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='Журнал событий';

CREATE TABLE EventJournalArc (
	EventID INT COMMENT 'Код',
	EquipID INT COMMENT 'Код записи',
	PointID INT COMMENT 'Код скважины',
	StatusID INT COMMENT 'Код статуса',
	State VARCHAR(40) COMMENT 'Состояние',
	Note VARCHAR(500) COMMENT 'Обоснование',
	EventDate DATETIME COMMENT 'Дата события',
	TokenID BIGINT COMMENT 'Код пользователя',
	UserArc BIGINT COMMENT 'Modified by',
	DateArc DATETIME COMMENT 'Modification time',
	ArcType VARCHAR(1) COMMENT 'ArcType',
	PRIMARY KEY (EventID, UserArc, DateArc)
) ENGINE=Innobase DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='Table for audit Журнал событий';


CREATE INDEX EventJournalEquipment ON EventJournal(EquipID);
ALTER TABLE EventJournal ADD CONSTRAINT EventJournal_Equipment FOREIGN KEY (EquipID) REFERENCES Equipment (EquipID);

CREATE INDEX EventJournalEquipmentPoint ON EventJournal(PointID);
ALTER TABLE EventJournal ADD CONSTRAINT EventJournal_EquipmentPoint FOREIGN KEY (PointID) REFERENCES EquipmentPoint (PointID);

CREATE INDEX EventJournalEStatus ON EventJournal(StatusID);
ALTER TABLE EventJournal ADD CONSTRAINT EventJournal_EStatus FOREIGN KEY (StatusID) REFERENCES EStatus (StatusID);

CREATE INDEX EventJournalGPNUser ON EventJournal(TokenID);
ALTER TABLE EventJournal ADD CONSTRAINT EventJournal_GPNUser FOREIGN KEY (TokenID) REFERENCES GPNUser (TokenID);


CREATE INDEX EventJournalEquipment ON EventJournal(EquipID);
ALTER TABLE EventJournal ADD CONSTRAINT EventJournal_Equipment FOREIGN KEY (EquipID) REFERENCES Equipment (EquipID);

CREATE INDEX EventJournalEquipmentPoint ON EventJournal(PointID);
ALTER TABLE EventJournal ADD CONSTRAINT EventJournal_EquipmentPoint FOREIGN KEY (PointID) REFERENCES EquipmentPoint (PointID);

CREATE INDEX EventJournalEStatus ON EventJournal(StatusID);
ALTER TABLE EventJournal ADD CONSTRAINT EventJournal_EStatus FOREIGN KEY (StatusID) REFERENCES EStatus (StatusID);

CREATE INDEX EventJournalGPNUser ON EventJournal(TokenID);
ALTER TABLE EventJournal ADD CONSTRAINT EventJournal_GPNUser FOREIGN KEY (TokenID) REFERENCES GPNUser (TokenID);

