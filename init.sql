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
INSERT INTO `Token` (TokenID, TokenName, TokenStatus, UserArc, DateArc, TokenType) VALUES 
(100110034,'Корупаева Марина Юрьевна','enabled',NULL,'0000-00-00 00:00:00','Users'),
(100110035,'Титова Нина Сергеевна','enabled',NULL,'0000-00-00 00:00:00','Users');
INSERT INTO `GPNUser` (TokenID, LastName, FirstName, SecondName, UStatus, UserArc, DateArc) VALUES 
(100110034,'Корупаева', 'Марина', 'Юрьевна','Активный',NULL,'0000-00-00 00:00:00'),
(100110035,'Титова', 'Нина', 'Сергеевна','Активный',NULL,'0000-00-00 00:00:00');

-- Users
INSERT INTO `Users` (TokenID, Login, Password, UserArc, DateArc) VALUES 
(100110034,'10034','123456',NULL,NULL),
(100110035,'10035','123456',NULL,NULL);

INSERT INTO `Member` ( TokenID, ChildTokenID, UserArc, DateArc ) VALUES
( 1, 100110034, -1, '0000-00-00 00:00:00' ),
( 1, 100110035, -1, '0000-00-00 00:00:00' );

INSERT INTO `Organization` ( Name, Address, LAddress, IsExecuter, IsProducer, IsShipper, UserArc, DateArc ) VALUES
( 'Подрядная организация', '1', '1', 'true', 'false', 'false', -1, '0000-00-00 00:00:00' ),
( 'Производитель', '1', '1', 'false', 'true', 'false', -1, '0000-00-00 00:00:00' ),
( 'Поставщик', '1', '1', 'false', 'false', 'true', -1, '0000-00-00 00:00:00' );

INSERT INTO `EquipmentKind` (Kind, Name, ProducerOrgID, UserArc, DateArc ) VALUES
( 'gate_valves', 'Задвижка', '3','-1','0000-00-00 00:00:00'),
( 'pomp', 'Насос', '3','-1','0000-00-00 00:00:00'),
( 'umbilical_cable', 'Шлангокабель', '3','-1','0000-00-00 00:00:00');
