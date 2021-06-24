#include "gpn.h"

#include "Services/Authorizer/authorizer_auto.h"
//#include "aofficeconnector-transform.h"
#include "dynwin/dynwin.h"


#include <sys/stat.h>

int AfterRegisterCallback(MTContainer* container) {
        return 0;
}


namespace GPN {

string getFullName(const GPNUser& user) {
        return Trim(Trim(user.LastName.get_value_or(""))+' '+
                Trim(user.FirstName.get_value_or(""))+' '+
                Trim(user.SecondName.get_value_or("")));
}

const Value GPNGPNUser::Get(const Int64 TokenID) {
	Value res;
	GPNUser aGPNUser;
	aGPNUser.TokenID=TokenID;
	aGPNUser.Select(rdb_);
	AUTHORIZER::Users user;
	user.TokenID = TokenID;
	user.Select( rdb_ );
	res["TokenID"]=aGPNUser.TokenID;
	res["LastName"]=aGPNUser.LastName;
	res["FirstName"]=aGPNUser.FirstName;
	res["SecondName"]=aGPNUser.SecondName;
	res["Login"]=user.Login;
	res["UStatus"]=aGPNUser.UStatus;

	//Формируем список групп
	AUTHORIZER::Member m;
	AUTHORIZER::Groups g;
	AUTHORIZER::Token gt;
	Selector gSel;
	gSel << gt->TokenID << gt->TokenName;
	gSel.Where( m->ChildTokenID == TokenID && m->TokenID == g->TokenID && g->TokenID == gt->TokenID );
	DataSet gData = gSel.Execute( rdb_ );
	Value GroupsList;
	int ind=0;
	while(gData.Fetch()) {
		GroupsList[ind]["TokenID"] = gt.TokenID;
		GroupsList[ind]["TokenName"] = gt.TokenName;
		++ind;
	}

	res["GroupsList"]=GroupsList;
	return res;
}


const Value GPNGPNUser::Add( const Str LastName, const Str FirstName, const Str SecondName, const Str Login, const Str Password, const Str PasswordConfirm, const Value GroupsList ) {

	// генерим TokenID
	int64_t TokenID;

        int Num;
        GPNUser au;
        Selector sel;
        sel << au->TokenID.Max().Bind(Num);
        DataSet data = sel.Execute(rdb_);
        if (data.Fetch() && Num > 0)
                TokenID = Num + 1;
        else
                TokenID = 100000000 + 1;

        // Проверяем что такого TokenID еще нет
	GPNUser aUser;
        Selector selE;
        selE << aUser->TokenID;
        selE.Where (aUser->TokenID == ValueOf(TokenID));
        while(selE.Execute(rdb_).Fetch()) {
                ++TokenID;
        }

        AddWithTokenID ((Int64)TokenID, LastName, FirstName, SecondName, Login, Password, PasswordConfirm, GroupsList );

        Value res;
        res ["TokenID"] = (int64_t)TokenID;
        res ["UserTokenID"] = SimpleThread::GetCurrentContext().GUserID();

        GPNUser u;
        u.TokenID = TokenID;
        u.Select(rdb_);
        string fullName=getFullName(u);

        AddMessage(Message("Пользователь ")<<fullName<<" зарегистрирован. ");
        if(Undefined(Login) || Login->empty()) AddMessage(Message("Без права доступа в Систему. "), Message::WARNING);
        else if(GroupsList.Size()==0) AddMessage(Message("Без права работать в Системе. "), Message::WARNING);
        return res;
}

const Value GPNGPNUser::AddWithTokenID (const Int64 TokenID, const Str LastName, const Str FirstName, const Str SecondName,
					 const Str Login, const Str Password, const Str PasswordConfirm, const Value GroupsList) {
        if(Undefined(LastName) || LastName->empty())
                throw Exception("Укажите Фамилию Пользователя. ");
        if(Undefined(FirstName) || FirstName->empty())
                throw Exception("Укажите Имя Пользователя. ");
        if(Password != PasswordConfirm)
                throw Exception("Пароли не совпадают");

        if (!(Undefined(Login) || Login->empty()))
        {
                AUTHORIZER::Users aUsers;
                Selector selL;
                selL << aUsers->TokenID;
                selL.Where (aUsers->Login == Login);
                if (selL.Execute(rdb_).Fetch())
                        throw Exception("Такой логин уже используется у другого Пользователя.");
        }


        // Добавляем пользователя
        GPNUser au;
        au.TokenID = TokenID;
        au.LastName=LastName;
        au.FirstName=FirstName;
        au.SecondName=SecondName;
        au.UStatus=USER_ACTIVE;
        string fullName=getFullName(au);
        au.Insert(rdb_);

        // Добавляем юзера

        AUTHORIZER::Token t;
        t.TokenID = TokenID;
        t.TokenName=fullName;
        t.TokenStatus=AUTHORIZER::TST_ENABLED;
        t.TokenType=AUTHORIZER::AUTHORIZER_TOKEN_USERS;
        t.Insert(rdb_);

        AUTHORIZER::Users u;
        u.TokenID = TokenID;
        u.Login=Login;
        u.Password=Password;
        u.Insert(rdb_);

        if(GroupsList.IsArray()) {
                AUTHORIZER::Member m;
                m.ChildTokenID=TokenID;

                for(unsigned idx=0; idx<GroupsList.Size(); ++idx) {
                        const Value& v=GroupsList[idx]["TokenID"];
                        if(v.Is<int64_t>("int64_t")) m.TokenID=v.As<int64_t>();
                        else m.TokenID=v.As<Int64>();
                        try {
                                m.Insert(rdb_);
                        } catch(...){}
                }
        }

        return ValueNULL;
}

const Value GPNGPNUser::Update(const Int64 TokenID, const optional<Str> LastName, const optional<Str> FirstName, const optional<Str> SecondName,
				const optional<Str> Login, const optional<Value> GroupsList, const optional<Str> UStatus) {
	Exception e((Message("Cannot update ") << Message("Пользователь").What() << ". ").What());
	try {
		GPNUser aGPNUser;aGPNUser.TokenID=TokenID;aGPNUser.Select(rdb_);
		AUTHORIZER::Users user;	user.TokenID = TokenID;	user.Select(rdb_);
		AUTHORIZER::Token token;token.TokenID = TokenID;token.Select(rdb_);
		bool FIOIsChanged( false );
		if(Defined(LastName)) {
			aGPNUser.LastName=*LastName;
			FIOIsChanged = true;
		}
		if(Defined(FirstName)) {
			aGPNUser.FirstName=*FirstName;
			FIOIsChanged = true;
		}
		if(Defined(SecondName)) {
			aGPNUser.SecondName=*SecondName;
			FIOIsChanged = true;
		}
		if(Defined(UStatus)) aGPNUser.UStatus=*UStatus;
		aGPNUser.Update(rdb_);
		AddMessage(Message()<<Message("Пользователь").What()<<" "<<aGPNUser.TokenID<<" updated. ");

		if( Defined( Login ) ) {
			user.Login = *Login;
		}
		user.Update( rdb_ );
		if( FIOIsChanged == true ) {
			token.TokenName = getFullName( aGPNUser );
		}
		token.Update( rdb_ );

		if(GroupsList) {
			if(GroupsList.get_value_or(Value()).IsArray()) {
				AUTHORIZER::Member m;

				Selector sel;
				sel << m;                       
				sel.Where(m->ChildTokenID==TokenID);
				DataSet ds = sel.Execute(rdb_);

				set<Int64> prevGroups;
				while (ds.Fetch()) {
					prevGroups.insert(m.TokenID);
				}

				m.ChildTokenID=TokenID;
				for(unsigned idx=0; idx<GroupsList->Size(); ++idx) {
					const Value& v=(*GroupsList)[idx]["TokenID"];
					Int64 gTokenID;
					if(v.Is<int64_t>("int64_t")) gTokenID=v.As<int64_t>();
					else gTokenID=v.As<Int64>();

					if( prevGroups.find( gTokenID ) == prevGroups.end() ) {
						m.TokenID = gTokenID;
						AUTHORIZER::AUTHORIZERMember::Add(m.TokenID, m.ChildTokenID);
					} else {
						prevGroups.erase( gTokenID );
					}
				}
				if( prevGroups.size() ) {
					for( set<Int64>::iterator it = prevGroups.begin(); it != prevGroups.end(); ++it) {
						AUTHORIZER::AUTHORIZERMember::Delete(*it, TokenID); 
					}
				}
			}
		}

		return Value();
	}
	catch (Exception& exc) {
		if(exc.ErrorCode() == RDBMS_ERR_DUPLICATE_ENTRY) {
			e << Message("Duplicate entry. ").What();
		}
		else {
			e << Message(exc.what()).What();
		}
		throw e;
	}
}


const Value GPNGPNUser::Activate ( const Int64 TokenID ) {
	GPNUser aGPNUser;	aGPNUser.TokenID=TokenID;	aGPNUser.Select(rdb_);
	aGPNUser.UStatus = USER_ACTIVE;
	aGPNUser.Update( rdb_ );

	AUTHORIZER::Token token; token.TokenID = TokenID; token.Select( rdb_ );
        token.TokenStatus = AUTHORIZER::TST_ENABLED;
        token.Update( rdb_ );
	return ValueNULL;
}

const Value GPNGPNUser::Block ( const Int64 TokenID ) {
	GPNUser aGPNUser;	aGPNUser.TokenID=TokenID;	aGPNUser.Select(rdb_);
	aGPNUser.UStatus = USER_BLOCK;
	aGPNUser.Update( rdb_ );

	AUTHORIZER::Token token; token.TokenID = TokenID; token.Select( rdb_ );
	token.TokenStatus = AUTHORIZER::TST_DISABLED;
	token.Update( rdb_ ); 
	return ValueNULL;
}

const Value GPNGPNUser::GPNUserListGet() {
	Data::DataList lr;
	lr.AddColumn("TokenID", Data::INT64);
	lr.AddColumn("FullName", Data::STRING);
	lr.AddColumn("Login", Data::STRING);
	lr.AddColumn("Groups", Data::STRING);
	lr.AddColumn("UStatus", Data::STRING, USER_STATUSValues() );
	GPNUser aGPNUser;
	AUTHORIZER::Token token;
	AUTHORIZER::Users user;
	string FullName;
	string Groups;
	lr.Bind(aGPNUser.TokenID, "TokenID");
	lr.Bind(FullName, "FullName");
	lr.Bind(user.Login, "Login");
	lr.Bind(Groups, "Groups");
	lr.Bind(aGPNUser.UStatus, "UStatus");

	//Формируем список групп
	AUTHORIZER::Member m;
	AUTHORIZER::Groups g;
	AUTHORIZER::Token gt;
	Selector gSel;
	gSel << gt->TokenName << m->ChildTokenID;
	gSel.Where( m->TokenID == g->TokenID && g->TokenID == gt->TokenID );
	DataSet gData = gSel.Execute( rdb_ );
	map<Int64, string> AllGroupsByUsers;
	while(gData.Fetch()) {
		if( AllGroupsByUsers.find(m.ChildTokenID) == AllGroupsByUsers.end() ) {
			AllGroupsByUsers[m.ChildTokenID] = gt.TokenName.get_value_or("");

		} else if( IsNotNull( gt.TokenName ) && gt.TokenName.get_value_or("") != "" ) {
			AllGroupsByUsers[m.ChildTokenID] += ( AllGroupsByUsers[m.ChildTokenID] != "" ? string(", "): "")
						 + gt.TokenName.get_value_or("");
		}
	}

	Selector sel;
	sel << aGPNUser->TokenID << aGPNUser->LastName << aGPNUser->FirstName << aGPNUser->SecondName 
		<< user->Login <<aGPNUser->UStatus;
	sel.Where( aGPNUser->TokenID == user->TokenID );
	DataSet data=sel.Execute(rdb_);
	while(data.Fetch()) {
        	FullName=getFullName(aGPNUser);
		Groups = AllGroupsByUsers[aGPNUser.TokenID ];	
		lr.AddRow();
	}
	Value res=lr;
	return res;
}

const Value GPNEquipment::EquipmentMenuListGet(const optional<Int> EquipKindID ) {
	Data::DataList lr;
	lr.AddColumn("EquipID", Data::INTEGER);
	lr.AddColumn("mcpContractID", Data::INTEGER);
	lr.AddColumn("mcpName", Data::STRING);
	lr.AddColumn("ETypeKind", Data::STRING, EQUIP_KINDValues());
	lr.AddColumn("ETypeName", Data::STRING);
	lr.AddColumn("InvNumber", Data::STRING);
	lr.AddColumn("EStatus", Data::STRING, EQUIPMENT_STATUSValues());
	lr.AddColumn("EState", Data::STRING, EQUIPMENT_STATEValues());
	Equipment aEquipment;
	Contract amcp;
	EquipmentKind aEType;
	lr.Bind(aEquipment.EquipID, "EquipID");
	lr.Bind(amcp.ContractID, "mcpContractID");
	lr.Bind(amcp.Name, "mcpName");
	lr.Bind(aEType.Kind, "ETypeKind");
	lr.Bind(aEType.Name, "ETypeName");
	lr.Bind(aEquipment.InvNumber, "InvNumber");
	lr.Bind(aEquipment.EStatus, "EStatus");
	lr.Bind(aEquipment.EState, "EState");
	Selector sel;
	sel << aEquipment->EquipID << aEquipment->InvNumber << aEquipment->EStatus << aEquipment->EState
		<< amcp->ContractID << amcp->Name << aEType->Kind << aEType->Name;
	sel.Where(aEquipment->EquipKindID==aEType->EquipKindID &&
		 ( Defined(EquipKindID) ? aEquipment->EquipKindID==*EquipKindID : Expression()) );
	DataSet data=sel.Execute(rdb_);
	while(data.Fetch()) lr.AddRow();
	Value res=lr;
	return res;
}


}
