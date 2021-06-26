
--CREATE TABLE Documents (
--	DocID INT NOT NULL AUTO_INCREMENT COMMENT 'Код документа',
--	Name VARCHAR(40) NOT NULL COMMENT 'Наименование',
--	AddDate DATETIME COMMENT 'Дата добавления',
--	CreateDate DATETIME COMMENT 'Дата',
--	AddByUserTokenID BIGINT COMMENT 'Token ID',
--	EquipID INT COMMENT 'Код записи',
--	FileBody LONGBLOB NOT NULL COMMENT 'Документ',
--	UserArc BIGINT COMMENT 'Modified by',
--	DateArc DATETIME COMMENT 'Modification time',
--	PRIMARY KEY (DocID)
--) ENGINE=Innobase DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='Документ';

--CREATE TABLE DocumentsArc (
--	DocID INT COMMENT 'Код документа',
--	Name VARCHAR(40) COMMENT 'Наименование',
--	AddDate DATETIME COMMENT 'Дата добавления',
--	CreateDate DATETIME COMMENT 'Дата',
--	AddByUserTokenID BIGINT COMMENT 'Token ID',
--	EquipID INT COMMENT 'Код записи',
--	FileBody LONGBLOB COMMENT 'Документ',
--	UserArc BIGINT COMMENT 'Modified by',
--	DateArc DATETIME COMMENT 'Modification time',
--	ArcType VARCHAR(1) COMMENT 'ArcType',
--	PRIMARY KEY (DocID, UserArc, DateArc)
--) ENGINE=Innobase DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='Table for audit Документ';

--CREATE INDEX DocumentsAddByUser ON Documents(AddByUserTokenID);
--ALTER TABLE Documents ADD CONSTRAINT Documents_AddByUser FOREIGN KEY (AddByUserTokenID) REFERENCES Users (TokenID);

--CREATE INDEX DocumentsEquipment ON Documents(EquipID);
--ALTER TABLE Documents ADD CONSTRAINT Documents_Equipment FOREIGN KEY (EquipID) REFERENCES Equipment (EquipID);
