#include "inc/apRecon.hpp"

using namespace ap;


Reconstructor::Reconstructor()
{
    proceso = new QProcess();

    // Conecto los callbacks
    this->connect(proceso, SIGNAL(readyReadStandardError()), this, SLOT(updateError()));
    this->connect(proceso, SIGNAL(readyReadStandardOutput()), this, SLOT(on_readyRead()));
    this->connect(proceso, SIGNAL(finished(int , QProcess::ExitStatus )), this, SLOT(on_procesoExit(int , QProcess::ExitStatus )));

    // conecto la signal de parser
    this->connect(this, SIGNAL(signal_finParser()), &loop_parser, SLOT(quit()));
    // conecto la signal de reconstruccion
    this->connect(this, SIGNAL(signal_finReconstruccion()), &loop_reconstruccion, SLOT(quit()));

    // Paths a dependencias
    path_APIRL = "../../../apirl-code-pet/build/cmd/";
    path_INTERFILES = "../../../interfiles/";
    path_PARSER = "../../../Parser/";
    path_Salida = QDir::currentPath()+"/Salidas/";

    // Archivos
    arch_recon = "-";
    arch_ini = "-";
    arch_sens = "-";
    arch_countskimm = "-";



    // Sinograma
    Cant_anillos = CANTANILLOS_BASE;
    Dif_anillos = MAXDIFANILLOS_BASE;
    Emax = EMAX_BASE;
    Emin = EMIN_BASE;
    Span = SPAM_BASE;
    cant_ang = CANTANGULOS_BASE;
    cant_rhos = CANTRHOS_BASE;
    max_Rho = MAXRHO_BASE;
    max_Z = MAXZ_BASE;

    // Archivo de parametros
    FOV_Axial = FOV_AXIAL_BASE;
    Min_dif_cab = MIN_DIF_CAB_BASE;
    Radio_FOV = RADIO_FOV_BASE;
    Radio_PET = RADIO_PET_BASE;
    zona_muerta = ZONA_MUERTA_BASE;
    iteraciones = ITERACIONES_BASE;

    // Server Paths
    SERVER_BASE             =   "/mnt/running/apirl-mediciones/";
    SERVER_ENCABEZADOS      =   "/mnt/running/apirl-mediciones/encabezados/";
    SERVER_ENTRADAS         =   "/mnt/running/apirl-mediciones/datos_Entrada/";
    SERVER_SALIDAS          =   "/mnt/running/apirl-mediciones/output_APIRL/";
    SERVER_APIRL            =   "/mnt/running/apirl-code-pet/build/cmd/";


}
Reconstructor::~Reconstructor()
{
    matar_procesos();
}

bool Reconstructor::SetearListasProcesos()
{

    if(parsear)
    {
        limite_ejecucion = INST_PARSEO;
    }
    else if(reconstruir & reconServer & SensibilidadPrecalculada)
    {
        limite_ejecucion = INST_RECON_SERVER+INST_SENS_SERVER;
    }
    else if(reconstruir & reconServer)
    {
        limite_ejecucion = INST_RECON_SERVER;
    }
    else if(reconstruir)
    {
        limite_ejecucion = INST_RECON_LOCAL;
    }
    else if(mostrar)
    {
        limite_ejecucion = INST_MOSTRAR;
    }

    programas = new QString[limite_ejecucion];
    listasparametros = new QStringList[limite_ejecucion];


}

bool Reconstructor::ResetearListasProcesos()
{

    // Seteo el flag de muerto
    muerto = 1;
    // Mato el proceso
    proceso->kill();
    muerto = 0;
    limite_ejecucion = 0;
    indice_armado_cola = 0;
    indice_ejecucion = 0;

    //delete(programas);
    //delete(listasparametros);

}


bool Reconstructor::Parsear()
{

    // Con el nuevo parseador del Doctor, se usa solo una instrucción (Si usted logra compilarlo)

    // Primero armo el archivo de parametros
    QString nombre_archivo_parametros;
    QTextStream stream_par;
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    time (&rawtime);
    timeinfo = localtime(&rawtime);

    nombre_archivo_parametros = path_Salida+Nombre_archivo+".ini";

    QFile file(nombre_archivo_parametros);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        consola->appendPlainText("Error al abrir/crear archivo de parametros de parseo");
        return -1;
    }

    stream_par.setDevice(&file);

    stream_par<<"; Archivo de parametros generado automaticamente por el AR-PET-Tec. Fecha: "<< asctime(timeinfo) <<endl;

    // Escribo el método del doctor, no se que hace acá
    stream_par<<"[MLEM]"<<endl;
    // Y el resto de sus definiciones, que como por el momento no hay definición de su archivo de reconstruccion
    // se generan con valores HARDCODEADOS D=
    stream_par<<"\t Tamano_Voxel_X = "<< 2.0 <<endl;
    stream_par<<"\t Tamano_Voxel_Y = "<< 2.0 <<endl;
    stream_par<<"\t Tamano_Voxel_Z = "<< 2.0 <<endl;
    stream_par<<"\t Cantidad_Voxeles_X = "<<200<<endl;
    stream_par<<"\t Cantidad_Voxeles_Y = "<<200<<endl;
    stream_par<<"\t Cantidad_Voxeles_Z = "<<200<<endl;

    // Este si lo tengo =D
    stream_par<<"\t Cantidad_Ciclos_MLEM="<<iteraciones<<endl;

    // Supongo q son las salidas de la reconstrucción, las harcodeo
    stream_par<<"\t Cantidad_Lors=18000000"<<endl;
    stream_par<<"\t Archivo_Lors=No_debo_mezclar_los_archivos_de_parametros.lor"<<endl;
    stream_par<<"\t Base_Archivos_Salida= No_debo_mezclar_los_archivos_de_parametros"<<endl;


    // Ahora esta parte parece ser lo que si nos importa

    // Con lo de abajo inicio la seccion donde van los campos para este programa
    // Al estar todo dentro de una sección, se puede utilizar el mismo archivo de configuración
    // para cada etapa del procesamiento
    stream_par<<"[PROCESAMIENTO TRAMA]"<<endl;
    // Archivo de entrada:
    stream_par<<"\t Archivo_Trama = "<<arch_recon<<endl;
    // Archivos de salidas (puedo obviar uno o los dos comentando la linea):
    stream_par<<"\t Archivo_Lors_Normalizadas = "<<path_Salida+Nombre_archivo+".lor"<<endl;
    //Archivo_Trama_Interpretada = Patron_Cobre_cat_2.dat
    stream_par<<"\t Base_Archivos_Interfile = "<<path_Salida+Nombre_archivo<<endl;
    stream_par<<"\t [[FILTROS]]"<<endl;
    stream_par<<"\t\t E_Min = "<<Emin<<endl;
    stream_par<<"\t\t E_Max = "<<Emax<<endl;
    stream_par<<"\t\t Zona_Muerta = "<<zona_muerta<<endl;
    stream_par<<"\t\t Delta_Detector = "<<Min_dif_cab<<endl;
    stream_par<<"\t [[ESTADISTICAS]]"<<endl;
    stream_par<<"\t\t guardar_tablas_de_estadistica=si"<<endl;
    stream_par<<"\t\t graficar_tablas_de_estadistica=si"<<endl;
    // Esta es la base de los archivos de estadistica, si no se indica se usara
    // el archivo de entrada como base (sacando los últimos 4 caracteres):
    stream_par<<"\t\t Base_Archivos_Estadisticas = "<<path_Salida+Nombre_archivo<<"_log_est"<<endl;
    stream_par<<"\t\t Bines_Energia = "<< EST_BINES_ENERGIA <<endl;
    stream_par<<"\t\t Ancho_Bin_Energia = "<< EST_ANCHO_BIN_ENERGIA <<endl;
    stream_par<<"\t\t Bines_Tiempo = "<< EST_BINES_TIEMPO <<endl;
    stream_par<<"\t\t Ancho_Bin_Tiempo = "<< EST_ANCHO_BIN_TIEMPO <<endl;
    stream_par<<"\t\t Bines_Espacial_X = "<< EST_BINES_ESPACIAL_X <<endl;
    stream_par<<"\t\t Bines_Espacial_Y = "<< EST_BINES_ESPACIAL_Y <<endl;
    stream_par<<"\t\t Ancho_Bin_Espacial = "<< EST_ANCHO_BIN_ESPACIALES<<endl;


    // Esta subseccion tiene parámetros que solo tienen interés si se pide las
    // Lors normalizadas para que sirva de entrada a la reconstruccion via mlem en modo lista
    // esto requiere filtrar las Lors que no pasen por el FOV y además representa las rectas
    // de la Lor de forma que el origen de la recta es una interseccion con el FOV y la pendiente
    // es tal que si r=origen+pendiente*a   a=1 en el otro cruce con el borde del FOV
    // => Requiere definir el FOV:
    stream_par<<"\t\t [[LOR NORMALIZADA]]"<<endl;
    stream_par<<"\t\t Cantidad_Voxeles_X = "<<200<<endl;
    stream_par<<"\t\t Cantidad_Voxeles_Y = "<<200<<endl;
    stream_par<<"\t\t Cantidad_Voxeles_Z = "<<200<<endl;
    stream_par<<"\t\t Tamano_Voxel_X = "<< 2.0 <<endl;
    stream_par<<"\t\t Tamano_Voxel_Y = "<< 2.0 <<endl;
    stream_par<<"\t\t Tamano_Voxel_Z = "<< 2.0 <<endl;
    stream_par<<"\t [[MICHELOGRAMA]]"<<endl;
    stream_par<<"\t\t Span = "<< Span <<endl;
    stream_par<<"\t\t Cantidad_Anillos = "<< Cant_anillos <<endl;
    stream_par<<"\t\t Cantidad_Angulos = "<< cant_ang <<endl;
    stream_par<<"\t\t Cantidad_Rhos = "<< cant_rhos <<endl;
    stream_par<<"\t\t Maxima_Diferencia_Entre_Anillos = "<< Dif_anillos <<endl;
    stream_par<<"\t\t Rho_Maxima = "<< max_Rho <<endl;
    stream_par<<"\t\t Z_Minima = "<< -max_Z <<endl;
    stream_par<<"\t\t Z_Maxima = "<< max_Z <<endl;


    //  ----- Paso unico, parseo
    // Armo la linea al programa
    programas[indice_armado_cola] = path_PARSER+"procesar_trama";

    // Armo los argumentos
    listasparametros[indice_armado_cola].append(nombre_archivo_parametros);
    indice_armado_cola++;

    // ejecuto
    proceso->start(programas[indice_ejecucion],listasparametros[indice_ejecucion]);

    // Ahora el archivo a recontruir dejo de ser el .raw y va a pasar a ser el .h33 que va ser generado
    arch_recon_orig = arch_recon;
    arch_recon = path_Salida+Nombre_archivo+".h33";

    return 1;
}
bool Reconstructor::Reconstruir()
{
    QString nombre_img_ini, nombre_sens_ini;
    QString aux_string;

    // ---- Primero armo el archivo de parametros

    // Abro archivo a editar
    QString nombre_archivo_parametros;
    QTextStream stream_par;
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    time (&rawtime);
    timeinfo = localtime(&rawtime);

    nombre_archivo_parametros = path_Salida+Nombre_archivo+".par";

    QFile file(nombre_archivo_parametros);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        consola->appendPlainText("Error al abrir/crear archivo de parametros");
        return -1;
    }

    stream_par.setDevice(&file);

    // Escribo el método (Que si o si debe ir primero)
    if (reconBackprojection) stream_par<<"Backproject Parameters :="<<endl;
    else if (reconMLEM) stream_par<<"MLEM Parameters :="<<endl;

    stream_par<<"; Archivo de parametros generado automaticamente por el AR-PET-Tec. Fecha: "<< asctime(timeinfo) <<endl;


    // Tipo de entrada, fija en Sinograma3DArPet
    stream_par<<"input type := Sinogram3DArPet"<<endl;

    // Escribo el tipo de entrada
    stream_par<<"input file := ";

    // Completo con el nombre de la entrada
    if (reconServer)
    {
        // Si es en el server allá, lo defino yo
        stream_par<<SERVER_ENTRADAS<<Nombre_archivo<<".h33"<<endl;
    }
    else
    {
            stream_par<<arch_recon<<endl;
    }


    // Escribo el initial estimate o el archivo de salida, que son lo mismo pero cada metodo lo llama como se le canta el culo
    if (reconBackprojection) stream_par<<"output image := ";
    else if (reconMLEM) stream_par<<"initial estimate := ";
    if (reconServer)
    {
        // Si lo mando al server la imagen inicial tambien la transfiero por su nombre
        nombre_img_ini =  arch_ini.split('.').first().split('/').last();
        stream_par<<SERVER_ENTRADAS<<nombre_img_ini<<".h33"<<endl;
    }
    else
        stream_par<<arch_ini<<endl;

    // Escribo el nombre de la salida, que tambien nes distinto... most intriging...
    if (reconBackprojection) stream_par<<"output filename := ";
    else if (reconMLEM) stream_par<<"output filename prefix := ";
    if (reconServer)
        stream_par<<SERVER_SALIDAS<<Nombre_archivo+"_out"<<endl;
    else
        stream_par<<path_Salida+Nombre_archivo+"_out"<<endl;

    // Para el caso de MLEM pongo la cantidad de iteracioens
    if (reconMLEM) stream_par<<"number of iterations := "<<iteraciones<<endl;

    // Seteamos el backproyector, fijo porque me place
    stream_par<<"backprojector := ArPetProjector"<<endl;

    // Para el caso de iterativos, tambien el forwardprojector
    if (reconMLEM) stream_par<<"forwardprojector := ArPetProjector"<<endl;

    // Escribimos los parametros del proyector
    stream_par<<"ArPet blind area (in mm) := "<<zona_muerta<<endl;
    stream_par<<"ArPet minimum difference between detectors := "<<Min_dif_cab<<endl;
    stream_par<<"cylindrical pet radius (in mm) := "<<Radio_PET<<endl;
    stream_par<<"radius fov (in mm) := "<<Radio_FOV<<endl;
    stream_par<<"axial fov (in mm) := "<<FOV_Axial<<endl;

    // Escribo parametros opcionales
    stream_par<<"enforce initial positivity condition := 0"<<endl;
    stream_par<<"save estimates at iteration intervals := 1"<<endl;
    stream_par<<"save estimated projections and backprojected image := 0"<<endl;

    // Si agregue imagen de sensibilidad la pongo
    if (SensibilidadPrecalculada)
    {
        if (reconServer)
        {
            // Si lo mando al server la imagen inicial tambien la transfiero por su nombre
            nombre_sens_ini =  arch_sens.split('.').first().split('/').last();
            stream_par<<"sensitivity filename := "<<SERVER_ENTRADAS<<nombre_sens_ini<<endl;
        }
        else
            stream_par<<"sensitivity filename := "<<arch_sens<<endl;
    }

    // Finalizo el archivo
    stream_par<<"END :="<<endl;


    if (reconServer)
    {
        // Como el server tiene otros path debo cambiar todas las lineas
        // de path del archivo de parametros e interfiles a el path relativo
        // del server

        // Si esta operacion esta encadenada con el parseo estamos en un problema,
        // para este punto es muy probable que el archivo que quiero modificar
        // no exista aún

        // Archivo a reconstruir
        // Copio al archivo
        QFile::copy(arch_recon, path_Salida+Nombre_archivo+".h33");
        aux_string = arch_recon.section(".",0,0);
        QFile::copy(aux_string+".i33", path_Salida+Nombre_archivo+".i33");
        QFile archivo_aux(path_Salida+Nombre_archivo+".h33");
        if (!archivo_aux.open(QIODevice::ReadWrite))
        {
            consola->appendPlainText("Error al abrir copia de archivo .h33 a reconstruir");
            return -1;
        }

        // Busco la linea que contiene el archivo .i33 y la remplazo por el nombre directo
        QString s;
        QTextStream t(&archivo_aux);
        while(!t.atEnd())
        {
            QString line = t.readLine();
            if(!line.contains("!name of data file := "))
                s.append(line + "\n");
            else
                s.append("!name of data file := " + Nombre_archivo + ".i33" + "\n");
        }
        archivo_aux.resize(0);
        t << s;
        archivo_aux.close();

        // Archivo inicial
        // Copio al archivo
        QFile::copy(arch_ini, path_Salida+nombre_img_ini+".h33");
        aux_string = arch_ini.section(".",0,0);
        QFile::copy(aux_string+".i33", path_Salida+nombre_img_ini+".i33");
        QFile archivo_aux_2(path_Salida+nombre_img_ini+".h33");
        if (!archivo_aux_2.open(QIODevice::ReadWrite))
        {
            consola->appendPlainText("Error al abrir copia de archivo .h33 de imagen inicial");
            return -1;
        }

        // Busco la linea que contiene el archivo .i33 y la remplazo por el nombre directo
        QString s_2;
        QTextStream t_2(&archivo_aux_2);
        while(!t_2.atEnd())
        {
            QString line = t_2.readLine();
            if(!line.contains("!name of data file := "))
                s_2.append(line + "\n");
            else
                s_2.append("!name of data file := " + nombre_img_ini + ".i33" + "\n");
        }
        archivo_aux_2.resize(0);
        t_2 << s_2;
        archivo_aux_2.close();

        if (SensibilidadPrecalculada)
        {
            // Archivo inicial
            // Copio al archivo
            QFile::copy(arch_sens, path_Salida+nombre_sens_ini+".h33");
            aux_string = arch_sens.section(".",0,0);
            QFile::copy(aux_string+".i33", path_Salida+nombre_sens_ini+".i33");
            QFile archivo_aux_3(path_Salida+nombre_sens_ini+".h33");
            if (!archivo_aux_3.open(QIODevice::ReadWrite))
            {
                consola->appendPlainText("Error al abrir copia de archivo .h33 de imagen de sensibilidad");
                return -1;
            }

            // Busco la linea que contiene el archivo .i33 y la remplazo por el nombre directo
            QString s_3;
            QTextStream t_3(&archivo_aux_3);
            while(!t_3.atEnd())
            {
                QString line = t_3.readLine();
                if(!line.contains("!name of data file := "))
                    s_3.append(line + "\n");
                else
                    s_3.append("!name of data file := " + nombre_sens_ini + ".i33" + "\n");
            }
            archivo_aux_3.resize(0);
            t_3 << s_3;
            archivo_aux_3.close();
        }


        //  ----- Si voy a reconstruir en el server copio los archivos ahi

        // Paso el archivo de parametros
        // Armo la linea al programa
        programas[indice_armado_cola] = "scp";
        // Armo los argumentos
        listasparametros[indice_armado_cola].append(nombre_archivo_parametros);
        listasparametros[indice_armado_cola].append(ip_SERVER+":"+SERVER_ENCABEZADOS);
        indice_armado_cola++;

        // Paso el h33 de la imagen a reconstruir
        // Armo la linea al programa
        programas[indice_armado_cola] = "scp";
        // Armo los argumentos
        listasparametros[indice_armado_cola].append(path_Salida+Nombre_archivo+".h33");
        listasparametros[indice_armado_cola].append(ip_SERVER+":"+SERVER_ENTRADAS);
        indice_armado_cola++;

        // Paso el i33 de la imagen a reconstruir
        aux_string = arch_recon.section(".",0,0);
        // Armo la linea al programa
        programas[indice_armado_cola] = "scp";
        // Armo los argumentos
        listasparametros[indice_armado_cola].append(aux_string+".i33");
        listasparametros[indice_armado_cola].append(ip_SERVER+":"+SERVER_ENTRADAS);
        indice_armado_cola++;



        // Paso el h33 de la imagen de sensibilidad
        // Armo la linea al programa
        programas[indice_armado_cola] = "scp";
        // Armo los argumentos
//        listasparametros[indice_armado_cola].append(path_Salida+nombre_sens_ini+".h33");
        aux_string = arch_ini.section(".",0,0);
        listasparametros[indice_armado_cola].append(path_Salida+nombre_img_ini+".h33");
        listasparametros[indice_armado_cola].append(ip_SERVER+":"+SERVER_ENTRADAS);
        indice_armado_cola++;

        // Paso el i33 de la imagen de sensibilidad
        // Armo la linea al programa
        programas[indice_armado_cola] = "scp";
        // Armo los argumentos
//        listasparametros[indice_armado_cola].append(aux_string+".i33");
        listasparametros[indice_armado_cola].append(path_Salida+nombre_img_ini+".i33");
        listasparametros[indice_armado_cola].append(ip_SERVER+":"+SERVER_ENTRADAS);
        indice_armado_cola++;

        if (SensibilidadPrecalculada)
        {
            // Paso el h33 de la imagen de sensibilidad
            // Armo la linea al programa
            programas[indice_armado_cola] = "scp";
            // Armo los argumentos
            listasparametros[indice_armado_cola].append(arch_sens);
            listasparametros[indice_armado_cola].append(ip_SERVER+":"+SERVER_ENTRADAS);
            indice_armado_cola++;

            // Paso el i33 de la imagen de sensibilidad
            aux_string = arch_sens.section(".",0,0);
            // Armo la linea al programa
            programas[indice_armado_cola] = "scp";
            // Armo los argumentos
            listasparametros[indice_armado_cola].append(aux_string+".i33");
            listasparametros[indice_armado_cola].append(ip_SERVER+":"+SERVER_ENTRADAS);
            indice_armado_cola++;
        }

        //  ----- Mando la reconstruccion

        // Armo la linea al programa
        programas[indice_armado_cola] = "ssh";

        // Armo los argumentos
        listasparametros[indice_armado_cola].append(ip_SERVER);

        /*
        aux_string = "\"cd "+SERVER_BASE+" ; "+SERVER_APIRL;
        if (reconBackprojection) listasparametros[indice_armado_cola].append(aux_string+"backproject "+SERVER_ENCABEZADOS+Nombre_archivo+".par \"");
        else if (reconMLEM) listasparametros[indice_armado_cola].append(aux_string+"MLEM  "+SERVER_ENCABEZADOS+Nombre_archivo+".par \"");
        */

        if (reconBackprojection) listasparametros[indice_armado_cola].append(SERVER_APIRL+"backproject "+SERVER_ENCABEZADOS+Nombre_archivo+".par");
        else if (reconMLEM) listasparametros[indice_armado_cola].append(SERVER_APIRL+"MLEM  "+SERVER_ENCABEZADOS+Nombre_archivo+".par");

        indice_armado_cola++;

        //  ----- Recupero la reconstruccion
        // Ahora la reconstruccion esta en el server y la tengo que traer.
        QString nombre_out_server;
        if (reconBackprojection) nombre_out_server = Nombre_archivo+"_out";
        else if (reconMLEM) nombre_out_server = Nombre_archivo+"_out_final";

        // Armo la linea al programa
        programas[indice_armado_cola] = "scp";
        // Armo los argumentos
        listasparametros[indice_armado_cola].append(ip_SERVER+":"+SERVER_SALIDAS+nombre_out_server+".h33");
        listasparametros[indice_armado_cola].append(path_Salida);
        indice_armado_cola++;

        // Armo la linea al programa
        programas[indice_armado_cola] = "scp";
        // Armo los argumentos
        listasparametros[indice_armado_cola].append(ip_SERVER+":"+SERVER_SALIDAS+nombre_out_server+".i33");
        listasparametros[indice_armado_cola].append(path_Salida);
        indice_armado_cola++;


    }
    else
    {
        //  ----- Mando la reconstruccion

        // Armo la linea al programa
        if (reconBackprojection) programas[indice_armado_cola] = path_APIRL+"backproject";
        else if (reconMLEM) programas[indice_armado_cola] = path_APIRL+"MLEM";

        // Armo los argumentos
        listasparametros[indice_armado_cola].append(nombre_archivo_parametros);
        indice_armado_cola++;
    }



    // Mando el proceso
    proceso->start(programas[indice_ejecucion],listasparametros[indice_ejecucion]);


    QString nombre_out_server;
    if (reconBackprojection) nombre_out_server = Nombre_archivo+"_out";
    else if (reconMLEM) nombre_out_server = Nombre_archivo+"_out_final";


    // Termine de reconstruir, ahora el archivo de entrada es:
    arch_recon = path_Salida+nombre_out_server+".h33";

    //
    if (reconServer)
    {
        //  ----- Acomodo el h33 para que sea legible
        // El h33 que llega del server tiene un path al i33 que es erroneo.
        // Peeeeero, ¡todabia no llego! (porque no lo reconstrui...), asi que me pongo a esperar...
        loop_reconstruccion.exec();

        // Copio al archivo
        QFile archivo_recuperado(path_Salida+nombre_out_server+".h33");
        if (!archivo_recuperado.open(QIODevice::ReadWrite))
        {
            consola->appendPlainText("Error al abrir el archivo .h33 reconstruido en el server");
            return -1;
        }


        // Busco la linea que contiene el archivo .i33 y la remplazo por el nombre directo
        QString s_out;
        QTextStream t_out(&archivo_recuperado);
        while(!t_out.atEnd())
        {
            QString line = t_out.readLine();
            if(!line.contains("!name of data file := "))
                s_out.append(line + "\n");
            else
                s_out.append("!name of data file := " + nombre_out_server + ".i33" + "\n");
        }
        archivo_recuperado.resize(0);
        t_out << s_out;
        archivo_recuperado.close();


    }







    return 1;
}

bool Reconstructor::Mostrar()
{
    //  ----- Mando al amide a hacer lo suyo
    programas[indice_armado_cola] = "amide";

    // Armo los argumentos
    listasparametros[indice_armado_cola].append(arch_recon);
    indice_armado_cola++;

    // Mando el proceso
    proceso->start(programas[indice_ejecucion],listasparametros[indice_ejecucion]);
}


bool Reconstructor::matar_procesos()
{
    // Seteo el flag de muerto
    muerto = 1;

    // Mato el proceso
    proceso->kill();

    // Reseteo la lista de ejecucion
    ResetearListasProcesos();


}

//---CALLBACKS

void Reconstructor::on_procesoExit(int flag_exit, QProcess::ExitStatus qt_exit)
{
    // Si llegue acá es porque finalizó algun programa, mando el proximo de la lista
    // si no es que ya estoy en el ultimo o esta muerto
    indice_ejecucion++;
    if (!muerto)
    {
        if (indice_ejecucion < limite_ejecucion)
            proceso->start(programas[indice_ejecucion],listasparametros[indice_ejecucion]);
        else
        {
            // Si estoy parseando y termine todo, emito la señal para desbloquear otros procesos
            if(parsear)
                emit signal_finParser();
            // Si estoy reconstruyendo, emito el final de la reconstruccion
            else if(reconstruir)
                emit signal_finReconstruccion();

            // Reseteo la lista de procesos
            ResetearListasProcesos();
        }
    }


}

void Reconstructor::on_readyRead()
{
    /*
    QByteArray datos =  proceso->readAllStandardOutput();
    QString sdatos = QString::fromUtf8(datos);
    consola->appendPlainText(sdatos);
    */
    QByteArray data = proceso->readAllStandardOutput();
    consola->appendPlainText(QString(data));
    qApp->processEvents();
}

void Reconstructor::updateError()
{

     QByteArray data = proceso->readAllStandardError();
     consola->appendPlainText(QString(data));
     qApp->processEvents();
}
