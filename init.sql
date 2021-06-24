-- CREATE UNIQUE INDEX ExamCardRegNoIDX ON ExamCard (RegNo, ExamCardType);

DELETE FROM TM_EventHandler;

-- Root privileges
-- call AddPrivileges(-1,'TaskMan','*','*',1,1,1,1,1,1);
call AddPrivileges(-1,'GPN','*','*',1,1,1,1,1,1);

insert into Token (TokenID, TokenName, TokenStatus, TokenType) values (1, "Администраторы", 'enabled', "Groups");
insert into Groups (TokenID) values(1);

insert into Token (TokenID, TokenName, TokenStatus, TokenType) values (2, "Пользователи", 'enabled', "Groups");
insert into Groups (TokenID) values(2);

call AddPrivileges(1,'GPN','*','*',1,1,1,1,1,1);
call AddPrivileges(2,'GPN','*','*',1,1,1,1,1,1);

-- Token
INSERT INTO `Organization` ( Name, Address, LAddress, UserArc, DateArc ) VALUES
( 'Газпром', '1', '1', -1, '0000-00-00 00:00:00' );

INSERT INTO `Token` (TokenID, TokenName, TokenStatus, UserArc, DateArc, TokenType) VALUES 
(100110034,'Корупаева Марина Юрьевна','enabled',NULL,'0000-00-00 00:00:00','Users'),
(100110035,'Титова Нина Сергеевна','enabled',NULL,'0000-00-00 00:00:00','Users');
INSERT INTO `GPNUser` (TokenID, LastName, FirstName, SecondName, OrgID, UStatus, UserArc, DateArc) VALUES 
(100110034,'Корупаева', 'Марина', 'Юрьевна', '1','Активный',NULL,'0000-00-00 00:00:00'),
(100110035,'Титова', 'Нина', 'Сергеевна', '1','Активный',NULL,'0000-00-00 00:00:00');

-- Users
INSERT INTO `Users` (TokenID, Login, Password, UserArc, DateArc) VALUES 
(100110034,'10034','123456',NULL,NULL),
(100110035,'10035','123456',NULL,NULL);

INSERT INTO `Member` ( TokenID, ChildTokenID, UserArc, DateArc ) VALUES
( 1, 100110034, -1, '0000-00-00 00:00:00' ),
( 1, 100110035, -1, '0000-00-00 00:00:00' );


INSERT INTO `EquipmentKind` (Kind, Name, UserArc, DateArc ) VALUES
( 'pomp', 'Насос','-1','0000-00-00 00:00:00');
