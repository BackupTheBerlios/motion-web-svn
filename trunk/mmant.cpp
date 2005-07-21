#include "main.h"

// Constructors/Destructors
// Methods


Mmant::Mmant () {
	del_force=true;
	EraseAll = -1;
	tmp_files = 0;
    Connection db;
};


int Mmant::OpenDB(const string &host, const string  &user, const string &password, const string& database, int port) {

try {
  db.set_Host(host);
  db.set_User(user);
  db.set_Password(password);
  db.set_Port(port);
  db.set_Timeout(60);
  
  db.connect();
  db.select_database(database);
  
} catch (ex_BadQuery& er){
	cerr << "Num.err: " << db.errnum() << endl;
  cerr << "Error: " << er.what() << " " << endl;
  return -1;
}
}

int Mmant::SetQuery(string s, string f) {
	q_where = s;
	q_where_count = f;
	personalized_query = true;
	DeleteDone = false;
	LOCK = false;
}

void Mmant::SetEraseAll(int cam) {
	EraseAll = cam;
}

int Mmant::SetFiles(char **files, int size) {

  string	file;
  m_size = size;  
  for (int i=1; i<size; i++) {
//    if (files[i] == NULL) {
//      cout << "FATAL ERROR, SetFiles";
//      return 1;
//    }
    file = files[i];
    m_files.push_back(file);
    m_fstatus.push_back(0);
    m_dbstatus.push_back(0);
  }
  return 1;
}

int Mmant::GetNumberFiles(int cam) {

	Query query = db.create_Query();
	if (personalized_query == true)
		query << "SELECT count(*) from security where camera='" << cam << "'" << q_where_count;
	else
		query << "SELECT count(*) from security where camera='" << cam << "'";
	Result_Use res = query.use();
	Row row = res.fetch_row();
	
//	cout << "Total number files: " << row[0] << endl;
	return row[0];
}

void Mmant::SetFilesFromDB(int cam) {

	// Limpiamos los vectores
	m_files.erase(m_files.begin(),m_files.end());
	m_fstatus.erase(m_fstatus.begin(), m_fstatus.end());
	m_dbstatus.erase(m_dbstatus.begin(), m_dbstatus.end());
	
	// Iniciamos el proceso
	Query query = db.create_Query();
	if (personalized_query == true)
		query << "SELECT * from security where camera='" << cam << "'" << q_where;
	else
		query << "SELECT * from security where camera='" << cam << "' limit 1000";
	Result_Store res = query.store();
//	cout << "Records found: " << res.size() << endl;
	Row row;
	Result_Store::iterator i;
	for (i = res.begin(); i != res.end(); i++) {
		row = *i;
		m_files.push_back(row[1]);
		m_fstatus.push_back(0);
		m_dbstatus.push_back(0);
	}
	m_size = m_files.size();
//	cout << "Añadidos " << m_size << " registros" << endl;
}

void * Mmant::DFilesThread (void *aa) {
	Mmant *zz = (Mmant *)aa;
	zz->LOCK = true;
//	int f_total,tmp_files=0;
//	float yo;
	zz->f_total = zz->GetNumberFiles(zz->EraseAll);
	cout << "Existen " << zz->f_total << " archivos en la base de datos" << endl;
	cout.precision(4);
 	do {
		zz->SetFilesFromDB(zz->EraseAll);
//		cout << "Eliminando HD" << endl;
		if ( zz->DeleteFilesHD() == -1 )
			cerr << "Hubo un error al eliminar archivos del disco duro" << endl;	
//		cout << "Eliminando DB" << endl;
 		if ( zz->DeleteFilesDB() == -1 )
			cerr << "Hubo un error al eliminar archivos de la base de datos" << endl;	
			zz->CheckDelete();
			zz->tmp_files = zz->GetNumberFiles(zz->EraseAll);
			
//			cout << m_percent << "%" << " Ahora: " << tmp_files << endl;
	} while ( zz->tmp_files != 0 && zz->DeleteDone == false);
	zz->DeleteDone = true;
	zz->LOCK = false;
	pthread_exit(NULL);
}

int Mmant::DeleteFiles() {
	if (EraseAll != -1 && LOCK == false) {
		pthread_t idThread;
		int err;
		Mmant *ptr = this;
		if (ptr == NULL) {
			cout << "ERROR" << endl;
			exit(-1);
		}
		err = pthread_create (&idThread, NULL, DFilesThread, ptr); 
		if ( err != 0 ) {
			cerr << "I can't create thread" << endl;
			return -1;
		}
		
		pthread_join(idThread, NULL);
//		sleep(1);
//		while ( DeleteDone == false ) {
//		}

//		SingleThread. Uncomment	
//		DFilesThread(ptr);
		
		return 0;
		
	}
}

void Mmant::DeleteStop(int s) {
	DeleteDone = true;
}

string Mmant::CheckDelete(int s) {
	char t1[20];
	char t2[20];
	m_percent = (100-((float)tmp_files/(float)f_total)*100);
	cout.precision(6);
	string a = "Vamos por el ";
	sprintf(t1,"%l",m_percent);
	printf ("T1: %l", t1);
	a += t1;
	a +=" y quedan ";
	sprintf(t2,"%l", tmp_files);
	a += t2;
	return a;
}
// Elimina archivos del disco duro.
int Mmant::DeleteFilesHD () {
	int fail = 0;

// DEBUG	
//	cout << "Eliminando " << m_size << " archivos" << endl;
	for (int i=0; i< m_size; i++) {
		if (m_fstatus[i] == 0) {
			if ( (unlink(m_files[i].c_str())) == -1) {
				cerr << "Warning: I can't delete file: " << m_files[i] << endl;
				fail = -1;
			}
			else
				m_fstatus[i] = 1;
		}
	}
	return fail;
}
bool Mmant::DeleteFilesDB () {
	int fail = 0;	
  try {
		Query query = db.create_Query();
		query = db.create_Query();
		Result_NoData res;
		for (int i=0; i<m_size; i++) {
			if ( m_dbstatus[i] == 0 && m_fstatus[i] == 1 || m_dbstatus[i] == 0 && del_force == true) { // Only if file is deleted.
				query << "DELETE from security where filename='" << m_files[i] << "'";
//				query << "delete from security where camera='WoW'";
				res = query.execute();
//				cout << "ROWS: " << res.get_rows_affected() << endl;
				if ( res.get_rows_affected() != 0 )
					m_dbstatus[i] = 1;
				else
					fail = -1;
				query = db.create_Query();
			}
		}
	
	} catch (ex_BadQuery& er){
		cerr << "Num.err: " << db.errnum() << endl;
		cerr << "Error: " << er.what() << " " << endl;
		fail = -1;
		return fail;
	}
//  Result_Store res = query.store();

  // Debug
//  cout << "Query: " << query.preview() << endl;
//  cout << "Records found: " << res.size() << endl;
  
//  Result_Use result = query.use();
//  Row row = result.fetch_row();
  
//  cout << row[1].data() << endl;
return fail;
}

int Mmant::CheckDelete(){

	for (int i=0; i<m_size; i++) {
		if (m_fstatus[i] == 0)
			cerr << "Archivo " << m_files[i] << " falló al eliminarse del HD" << endl;
		if (m_dbstatus[i] == 0)
			cerr << "Archivo " << m_files[i] << " falló al eliminarse de la DB" << endl;
//		cout << m_files[i] << " - " << m_fstatus[i] << "-" << m_dbstatus[i] << endl;
	}
}

