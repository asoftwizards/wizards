#include "gpn.h"
//#include "stamppdf.h"

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

Int GOrgID( ) {
	Int GUserID = SimpleThread::GetCurrentContext().GUserID();
	GPNUser aUser;
	aUser.TokenID=GUserID;
	aUser.Select(rdb_);       
	return aUser.OrgID;

}

const Value GPNGPNUser::Get(const Int64 TokenID) {
	Value res;
	GPNUser aGPNUser;
	aGPNUser.TokenID=TokenID;
	aGPNUser.Select(rdb_);
	Organization aOrganization( aGPNUser.OrgID );
	aOrganization.Select( rdb_ );
	AUTHORIZER::Users user;
	user.TokenID = TokenID;
	user.Select( rdb_ );
	res["TokenID"]=aGPNUser.TokenID;
	res["LastName"]=aGPNUser.LastName;
	res["FirstName"]=aGPNUser.FirstName;
	res["SecondName"]=aGPNUser.SecondName;
	res["Name"]=aOrganization.Name;
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


const Value GPNGPNUser::Add( const Str LastName, const Str FirstName, const Str SecondName, const Int OrgID,
			 const Str Login, const Str Password, const Str PasswordConfirm, const Value GroupsList ) {

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

        AddWithTokenID ((Int64)TokenID, LastName, FirstName, SecondName, OrgID, Login, Password, PasswordConfirm, GroupsList );

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

const Value GPNGPNUser::AddWithTokenID (const Int64 TokenID, const Str LastName, const Str FirstName, const Str SecondName, const Int OrgID,
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
	au.OrgID = OrgID;
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
				const optional<Int> OrgID, const optional<Str> Login, const optional<Value> GroupsList, const optional<Str> UStatus) {
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
		if( Defined( OrgID ) ) {
			aGPNUser.OrgID = *OrgID;
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

const Value GPNGPNUser::GPNUserListGet( const optional<Int> OrgID ) {
	Data::DataList lr;
	lr.AddColumn("TokenID", Data::INT64);
	lr.AddColumn("FullName", Data::STRING);
	lr.AddColumn("Name", Data::STRING );
	lr.AddColumn("Login", Data::STRING);
	lr.AddColumn("Groups", Data::STRING);
	lr.AddColumn("UStatus", Data::STRING, USER_STATUSValues() );
	GPNUser aGPNUser;
	Organization aOrganization;
	AUTHORIZER::Token token;
	AUTHORIZER::Users user;
	string FullName;
	string Groups;
	lr.Bind(aGPNUser.TokenID, "TokenID");
	lr.Bind(FullName, "FullName");
	lr.Bind(aOrganization.Name, "Name");
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
		<< aOrganization->Name << user->Login <<aGPNUser->UStatus;
	sel.From(aGPNUser).LeftJoin( aOrganization ).On( aGPNUser->OrgID == aOrganization->OrgID )
		.Join( user ).On( aGPNUser->TokenID == user->TokenID );
	sel.Where( (Defined( OrgID ) ? aGPNUser->OrgID == *OrgID: Expression() ) );
	DataSet data=sel.Execute(rdb_);
	while(data.Fetch()) {
        	FullName=getFullName(aGPNUser);
		Groups = AllGroupsByUsers[aGPNUser.TokenID ];	
		lr.AddRow();
	}
	Value res=lr;
	return res;
}

const Value GPNEquipment::EquipmentMenuListGet(const optional<Int> EquipKindID) {
	Data::DataList lr;
	lr.AddColumn("EquipID", Data::INTEGER);
	lr.AddColumn("ETypeKind", Data::STRING, EQUIP_KINDValues());
	lr.AddColumn("ETypeName", Data::STRING);
	lr.AddColumn("InvNumber", Data::STRING);
	lr.AddColumn("Name", Data::STRING);
	lr.AddColumn("EState", Data::STRING, EQUIPMENT_STATEValues());
	Equipment aEquipment;
	EquipmentKind aEType;
	EStatus aEStatus;
	lr.Bind(aEquipment.EquipID, "EquipID");
	lr.Bind(aEType.Kind, "ETypeKind");
	lr.Bind(aEType.Name, "ETypeName");
	lr.Bind(aEquipment.InvNumber, "InvNumber");
	lr.Bind(aEStatus.Name, "Name");
	lr.Bind(aEquipment.EState, "EState");

	Int GOrganization = GOrgID();

	Selector sel;
	sel << aEquipment->EquipID << aEquipment->InvNumber << aEStatus->Name << aEquipment->EState
		 << aEType->Kind << aEType->Name;
	sel.Where(aEquipment->EquipKindID==aEType->EquipKindID && aEquipment->StatusID == aEStatus->StatusID &&
		 ( Defined(EquipKindID) ? aEquipment->EquipKindID==*EquipKindID : Expression()) &&
		 ( IsNotNull( GOrganization ) ? aEquipment->OwnerOrgID == GOrganization : Expression() ));
	DataSet data=sel.Execute(rdb_);
	while(data.Fetch()) lr.AddRow();
	Value res=lr;
	return res;
}

const Value GPNContract::ContractListGet(const optional<Int> ExecuterOrgID) {
	Data::DataList lr;
	lr.AddColumn("ContractID", Data::INTEGER);
	lr.AddColumn("Name", Data::STRING);
	lr.AddColumn("BDate", Data::DATETIME);
	lr.AddColumn("EDate", Data::DATETIME);
	lr.AddColumn("executerName", Data::STRING);
	Contract aContract;
	Organization aexecuter;
	lr.Bind(aContract.ContractID, "ContractID");
	lr.Bind(aContract.Name, "Name");
	lr.Bind(aContract.BDate, "BDate");
	lr.Bind(aContract.EDate, "EDate");
	lr.Bind(aexecuter.Name, "executerName");
	Int GOrganization = GOrgID();
	Selector sel;
	sel << aContract->ContractID << aContract->Name << aContract->BDate << aContract->EDate << aexecuter->Name;
	sel.LeftJoin(aexecuter).On(aContract->ExecuterOrgID==aexecuter->OrgID);
	sel.Where( (Defined(ExecuterOrgID) ? aContract->ExecuterOrgID==*ExecuterOrgID : Expression()) &&
			( IsNotNull(GOrganization) ? aContract->ExecuterOrgID == GOrganization: Expression() ));
	DataSet data=sel.Execute(rdb_);
	while(data.Fetch()) lr.AddRow();
	Value res=lr;
	return res;
}

const Value GPNDocuments::Add( const BlobAccessor DocFile, const Int EquipID, const ADate CreateDate ) {
	Int CurrUserID = SimpleThread::GetCurrentContext().GUserID();

	Exception e((Message("Cannot add ") << Message("Документ").What() << ". ").What());
	bool error=false;
	error |= Defined(EquipID)  && !__CheckIDExists(Equipment()->EquipID, *EquipID, "Оборудование", e, rdb_);
	if(error) {
		throw(e);
	}

	if(DocFile.IsNull() || DocFile.Name().empty()) {
		throw Exception("Документ не выбран");
	}

	try {
		Documents aDocuments;
		aDocuments.Name=DocFile.Name();
		ShPtr<Blob> _File = DocFile.BlobData();
		aDocuments.FileBody=(*_File);

		aDocuments.AddDate=SimpleThread::GetCurrentContext().GUserTime().date();
		aDocuments.CreateDate=CreateDate;
		aDocuments.AddByUserTokenID=CurrUserID;
		aDocuments.EquipID=EquipID;
		Int sk=aDocuments.Insert(rdb_);
		Value res;
		res["DocID"]=sk;
		AddMessage(Message()<<Message("Документ").What()<<" "<<sk<<" added. ");
		return res;
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


const Value GPNDocuments::DocumentsByEquipListGet( const Int EquipID ) {
	Data::DataList lr;
	lr.AddColumn("DocID", Data::INTEGER);
	lr.AddColumn("Name", Data::STRING);
	lr.AddColumn("CreateDate", Data::DATETIME);
	lr.AddColumn("AddByUser", Data::STRING);


	Documents aDocuments;
	AUTHORIZER::Users addU;
	lr.Bind(aDocuments.DocID, "DocID");
	lr.Bind(aDocuments.Name, "Name");
	lr.Bind(aDocuments.CreateDate, "CreateDate");
	lr.Bind(addU.Login, "AddByUser");

	if( IsNotNull( EquipID ) ) {
		Selector sel;
		sel << aDocuments->DocID << aDocuments->Name << aDocuments->CreateDate;
		sel.LeftJoin( addU ).On( aDocuments->AddByUserTokenID == addU->TokenID );
		sel.Where( aDocuments->EquipID==EquipID );
		DataSet data=sel.Execute(rdb_);
		while(data.Fetch()) lr.AddRow();
	}
	Log(0) << "DocumentsByNIDListGet end" << endl;
	Value res=lr;
	return res;
}

const Value GPNDocuments::Download(const Int DocID ) {
        if (!Defined(DocID)) return BlobFile();
        Documents doc(DocID);
        if (!doc.Select(rdb_)) throw Exception("Документ ") << DocID << " не найден. ";

        BlobFile result;
        if (Defined(doc.Name)) result.SetFileName(*doc.Name );
        result.SetExpireTime(SimpleThread::GetCurrentContext().GUserTime());
        result.Content() = doc.FileBody;

        return result;
}

const Value GPNDocuments::Delete(const Value DocsArray ) {
        Int DocID;
        unsigned int max_i = ( DocsArray.IsArray() ? DocsArray.Size() : 1 );
        for( unsigned i=0; i < max_i; ++i ) {
                Int DocID = ( DocsArray.IsArray() ?
                        ( DocsArray[i]["DocID"].As<int>()) :
                        ( DocsArray.As<int>() )
                );
                if( IsNotNull( DocID ) ) {
                                Documents aRecord( DocID );
                                aRecord.Delete( rdb_ );
				AddMessage( Message() << Message("Документ").What()<<" "<<DocID<<" deleted. ");

                }
        }

        return Value();
}



/*const Value GPNDocuments::PrintDoc(const Int DocID ) {
        Documents aRecord( DocID );
        aRecord.Select( rdb_ );

        string fullDocName = aRecord.Name.get_value_or("");
        string dName = fullDocName.substr( 0, fullDocName.size() - 4 );
        string sType = fullDocName.substr(fullDocName.size() - 3);
        string tType = "pdf";

        string dn="/tmp/Converter-"+ToString( time(0) );
        mkdir(dn.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        string outfilename = dn + "/out.pdf" ;//"out.pdf";
        string infilename = dn + "/in."+ sType;//"in."+sType;

        WriteFile(aRecord.FileBody, infilename);

        ::system((string("touch ") + outfilename).c_str());

        if( sType == "doc" ) {
                Log(0) << "Transform doc:" << fullDocName << " to PDF" << endl;
        }
        else if (sType == "xls" ) {
                Log(0) << "Transform xls:" << fullDocName << " to PDF" << endl;

        } else throw Exception("Недопустимый формат документа");

        DLL::LoadDLL("libaofficeconnector-transform");
        void* funcPtr = DLL::GetObject("libaofficeconnector-transform", "AOfficeInstance");
        if(funcPtr != NULL) {
                Log(5) << "libaofficeconnector-transform loading..." << endl;
                AO::OfficeTransformProvider* (*libFunc)( ) = reinterpret_cast<AO::OfficeTransformProvider* (*)( )>(funcPtr);
                AO::OfficeTransformProvider* otpObj = libFunc();

                otpObj->SetSourceName( infilename );
                otpObj->SetTargetName( outfilename );
                otpObj->SetTargetType( tType );
                if( !otpObj->FileTransform() ) {
                        Log(2) << "Error in FileTransform..." << endl;
                }
                delete otpObj;
        }

        BlobFile result;
        result.SetFileName(dName+"_stamped.pdf");
        result.SetExpireTime(SimpleThread::GetCurrentContext().GUserTime());

        AUTHORIZER::Token user( SimpleThread::GetCurrentContext().GUserID() );
        user.Select(rdb_ );
        Effi::Value textStamp;
        textStamp["static"][0]="ДОКУМЕНТ";
        textStamp["sign"][0] = string("Распечатал: ")+user.TokenName.get_value_or("");
        textStamp["sign"][1] = "";
        textStamp["sign"][2] = ToString(SimpleThread::GetCurrentContext().GUserTime());

        Effi::Blob infile=ReadFile(outfilename);
        Effi::ShPtr<Effi::StampToPDF> spdf;
        spdf = new Effi::StampToPDF(infile);
        spdf->SetTextStamp(textStamp);
        Effi::Blob outFile = spdf->Process();

        result.Content() = outFile;

        return result;
}
*/
const Value GPNEStatus::EStatusNextListGet( const Int StatusID ) {
	return EStatusListGet();
}

}
