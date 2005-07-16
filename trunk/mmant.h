#ifndef MMANT_H
#define MMANT_H

using namespace mysqlcppapi;
using namespace std;
// Class Mmant
// Clase encargada de realizar las funciones de mantenimiento de Motion.
class Mmant {
// Public stuff
public:
	// Constructors
	// Empty Constructor
    	bool del_force;
	bool LOCK;	// Flag looking jobs
	
	Mmant ();
	// Se le pasa un array con los archivos.
	int SetFiles(char **files, int size);
	// Activamos el modo de eliminación de todos los datos de una cámara.
	void SetEraseAll(int cam);
	int OpenDB (const string& host, const string& user, const string& password, const string& database, int port=3306);
	// Borra archivos del disco duro y base de datos.
	int DeleteFiles ();
	int SetQuery(string s, string f);   
	// Signal Catcher
	void DeleteStop(int);
	void CheckDelete(int); 
    
// Protected stuff
protected:
    // Fields
    // Constructors
    // Accessor Methods
    // Operations
// Private stuff
private:
    // Fields
    // DB Object
    Connection db;

	// Flags
	bool personalized_query;
	int EraseAll; //Flag y número de cámara.
	bool DeleteDone; // Flag que indica al padre que la eliminación se ha terminado
	// Dinamic matrix
	string q_where; // Después del where cam='1'.... Recordar poner el limit 1000
	string q_where_count; // String para el count(*) después del where tb;	
	vector<string> m_files;
	vector<int> m_fstatus;
	vector<int> m_dbstatus;
	int m_size;    // Size of vector with files.
	
	//Inside DFilesThread
	float m_percent, f_total, tmp_files; // Tanto por ciento realizado.
	
    //Thread con la función de eliminación.
	//Thread function
    static void * DFilesThread (void *);
	
    // @param cam Número de la cámara a eliminar
    // Rellena los vectores con los archivos de la base de datos.
    void SetFilesFromDB(int cam);    
	
    // Elimina archivos del disco duro.
    int  DeleteFilesHD ();
    // 
    // @param paths Nombre de las rutas a buscar en sql
    // @param status matriz con el status de cada operación.
    bool DeleteFilesDB ();
    
    // Verifica la eliminación correcta.
    int CheckDelete(); 
 
    // Devuelve el número de registros de la DB.
    int GetNumberFiles(int cam);
    
};
#endif //MMANT_H

