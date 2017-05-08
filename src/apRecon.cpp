#include "inc/apRecon.hpp"

using namespace ap;


Reconstructor::Reconstructor()
{
    proceso = new QProcess();

    // Conecto los callbacks
    this->connect(proceso, SIGNAL(readyReadStandardError()), this, SLOT(updateError()));
    this->connect(proceso, SIGNAL(readyReadStandardOutput()), this, SLOT(on_readyRead()));
    this->connect(proceso, SIGNAL(finished(int , QProcess::ExitStatus )), this, SLOT(on_procesoExit(int , QProcess::ExitStatus )));

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

}

bool Reconstructor::SetearListasProcesos()
{

    if (parsear & reconstruir & reconServer)
    {
        limite_ejecucion = 6;
    }
    else if (parsear & reconstruir)
    {
        limite_ejecucion = 5;
    }
    else if(parsear)
    {
        limite_ejecucion = 4;
    }
    else if(reconstruir)
    {
        limite_ejecucion = 1;
    }
    programas = new QString[limite_ejecucion];
    listasparametros = new QStringList[limite_ejecucion];

}

bool Reconstructor::ResetearListasProcesos()
{

    limite_ejecucion = 0;
    indice_armado_cola = 0;
    indice_ejecucion = 0;
    //delete(programas);
    //delete(listasparametros);

}


bool Reconstructor::Parsear()
{

    // Voy cargando la lista

    //  ----- Paso 1, parseo
    // Armo la linea al programa
    programas[indice_armado_cola] = path_PARSER+"parsear";

    // Armo los argumentos
    listasparametros[indice_armado_cola].append(arch_recon);
    listasparametros[indice_armado_cola].append(path_Salida+Nombre_archivo+".dat");
    indice_armado_cola++;

    //  ----- Paso 2, analizo las coincidencias
    // Armo la linea al programa
    programas[indice_armado_cola] = path_INTERFILES+"AnalizoCoincidenciasPET.pl";

    // Armo los argumentos
    listasparametros[indice_armado_cola].append(path_INTERFILES+"AnalizoCoincidenciasPET.pl");
    listasparametros[indice_armado_cola].append("--Emin");
    listasparametros[indice_armado_cola].append(QString::number(Emin));
    listasparametros[indice_armado_cola].append("--Emax");
    listasparametros[indice_armado_cola].append(QString::number(Emax));
    listasparametros[indice_armado_cola].append("--in");
    listasparametros[indice_armado_cola].append(path_Salida+Nombre_archivo+".dat");
    listasparametros[indice_armado_cola].append("--out");
    listasparametros[indice_armado_cola].append(path_Salida+Nombre_archivo+".lor");
    indice_armado_cola++;


    //  ----- Paso 3, Armo el michelograma
    // Armo la linea al programa
    programas[indice_armado_cola] = path_INTERFILES+"GenerarMichelograma.pl";

    // Armo los argumentos
    listasparametros[indice_armado_cola].append("--in");
    listasparametros[indice_armado_cola].append(path_Salida+Nombre_archivo+".lor");
    listasparametros[indice_armado_cola].append("--out");
    listasparametros[indice_armado_cola].append(path_Salida+Nombre_archivo+".i33");
    listasparametros[indice_armado_cola].append("--CantAnillos");
    listasparametros[indice_armado_cola].append(QString::number(Cant_anillos));
    listasparametros[indice_armado_cola].append("--Span");
    listasparametros[indice_armado_cola].append(QString::number(Span));
    listasparametros[indice_armado_cola].append("--MaxDifAnillos");
    listasparametros[indice_armado_cola].append(QString::number(Dif_anillos));
    listasparametros[indice_armado_cola].append("--CantAngulos");
    listasparametros[indice_armado_cola].append(QString::number(cant_ang));
    listasparametros[indice_armado_cola].append("--CantRhos");
    listasparametros[indice_armado_cola].append(QString::number(cant_rhos));
    listasparametros[indice_armado_cola].append("--MaxRho");
    listasparametros[indice_armado_cola].append(QString::number(max_Rho));
    listasparametros[indice_armado_cola].append("--MaxZ");
    listasparametros[indice_armado_cola].append(QString::number(max_Z));
    indice_armado_cola++;

    //  ----- Paso 3, Armo el header del michelograma
    // Armo la linea al programa
    programas[indice_armado_cola] = path_INTERFILES+"GenerarEncabezadoInterfile.pl";

    // Armo los argumentos
    listasparametros[indice_armado_cola].append("--archivo_datos");
    listasparametros[indice_armado_cola].append(path_Salida+Nombre_archivo+".i33");
    listasparametros[indice_armado_cola].append("--out");
    listasparametros[indice_armado_cola].append(path_Salida+Nombre_archivo+".h33");
    listasparametros[indice_armado_cola].append("--CantAnillos");
    listasparametros[indice_armado_cola].append(QString::number(Cant_anillos));
    listasparametros[indice_armado_cola].append("--Span");
    listasparametros[indice_armado_cola].append(QString::number(Span));
    listasparametros[indice_armado_cola].append("--MaxDifAnillos");
    listasparametros[indice_armado_cola].append(QString::number(Dif_anillos));
    listasparametros[indice_armado_cola].append("--CantAngulos");
    listasparametros[indice_armado_cola].append(QString::number(cant_ang));
    listasparametros[indice_armado_cola].append("--CantRhos");
    listasparametros[indice_armado_cola].append(QString::number(cant_rhos));
    listasparametros[indice_armado_cola].append("--MaxRho");
    listasparametros[indice_armado_cola].append(QString::number(max_Rho));
    listasparametros[indice_armado_cola].append("--MaxZ");
    listasparametros[indice_armado_cola].append(QString::number(max_Z));
    indice_armado_cola++;


    // ejecuto el primer proceso de la lista, los sucesivos procesos se ejecutaran en el callback de finalizacion
    proceso->start(programas[indice_ejecucion],listasparametros[indice_ejecucion]);


    return 1;
}
bool Reconstructor::Reconstruir()
{

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

    stream_par<<"; Interfile generado automaticamente por el AR-PET-Tec. Fecha: "<< asctime(timeinfo) <<endl;


    // Tipo de entrada, fija en Sinograma3DArPet
    stream_par<<"input type := Sinogram3DArPet"<<endl;

    // Escribo el tipo de entrada
    stream_par<<"input file := ";

    // Completo con el nombre de la entrada
    if (parsear)
    {
        // Si vengo de parsear el nombre lo puse yo
        stream_par<<path_Salida+Nombre_archivo+".h33"<<endl;

    }
    else
    {
        // Sino no parseo, entra lo que pasaron de entrada
        stream_par<<arch_recon<<endl;
    }

    // Escribo el initial estimate o el archivo de salida, que son lo mismo pero cada metodo lo llama como se le canta el culo
    if (reconBackprojection) stream_par<<"output image := ";
    else if (reconMLEM) stream_par<<"initial estimate := ";
    stream_par<<arch_ini<<endl;

    // Escribo el nombre de la salida, que tambien nes distinto... most intriging...
    if (reconBackprojection) stream_par<<"output filename := ";
    else if (reconMLEM) stream_par<<"output filename prefix := ";
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
    if (SensibilidadPrecalculada) stream_par<<"sensitivity filename := "<<arch_sens<<endl;

    // Finalizo el archivo
    stream_par<<"END :="<<endl;




    //  ----- Mando la reconstruccion

    // Armo la linea al programa
    if (reconBackprojection) programas[indice_armado_cola] = path_APIRL+"backproject";
    else if (reconMLEM) programas[indice_armado_cola] = path_APIRL+"MLEM";

    // Armo los argumentos
    listasparametros[indice_armado_cola].append(nombre_archivo_parametros);
    indice_armado_cola++;


    if (!parsear)
    {
        // Si no parsie, arranco la ejecucion, sino viene funcionando.
        proceso->start(programas[indice_ejecucion],listasparametros[indice_ejecucion]);
    }










    return 1;
}

//---CALLBACKS

void Reconstructor::on_procesoExit(int flag_exit, QProcess::ExitStatus qt_exit)
{
    // Si llegue acá es porque finalizó algun programa, mando el proximo de la lista
    // si no es que ya estoy en el ultimo
    indice_ejecucion++;
    if (indice_ejecucion < limite_ejecucion)
        proceso->start(programas[indice_ejecucion],listasparametros[indice_ejecucion]);
    else
        ResetearListasProcesos();
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
