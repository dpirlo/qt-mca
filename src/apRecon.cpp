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
    path_Salida = "Salidas/";

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

}

bool Reconstructor::SetearListasProcesos()
{

    if (parsear & (reconMLEM | reconBackprojection))
    {
        limite_ejecucion = 5;
    }
    else
    {
        limite_ejecucion = 4;
    }
    programas = new QString[limite_ejecucion];
    listasparametros = new QStringList[limite_ejecucion];

}


bool Reconstructor::Parsear()
{

    // Voy cargando la lista

    //  ----- Paso 1, parseo
    // Armo la linea al programa
    programas[0] = path_PARSER+"parsear";

    // Armo los argumentos
    listasparametros[0].append(arch_recon);
    listasparametros[0].append(QDir::currentPath()+"/"+path_Salida+Nombre_archivo+".dat");

    //  ----- Paso 2, analizo las coincidencias
    // Armo la linea al programa
    programas[1] = path_INTERFILES+"AnalizoCoincidenciasPET.pl";

    // Armo los argumentos
    listasparametros[1].append(path_INTERFILES+"AnalizoCoincidenciasPET.pl");
    listasparametros[1].append("--Emin");
    listasparametros[1].append(QString::number(Emin));
    listasparametros[1].append("--Emax");
    listasparametros[1].append(QString::number(Emax));
    listasparametros[1].append("--in");
    listasparametros[1].append(QDir::currentPath()+"/"+path_Salida+Nombre_archivo+".dat");
    listasparametros[1].append("--out");
    listasparametros[1].append(QDir::currentPath()+"/"+path_Salida+Nombre_archivo+".lor");


    //  ----- Paso 3, Armo el michelograma
    // Armo la linea al programa
    programas[2] = path_INTERFILES+"GenerarMichelograma.pl";

    // Armo los argumentos
    listasparametros[2].append("--in");
    listasparametros[2].append(QDir::currentPath()+"/"+path_Salida+Nombre_archivo+".lor");
    listasparametros[2].append("--out");
    listasparametros[2].append(QDir::currentPath()+"/"+path_Salida+Nombre_archivo+".i33");
    listasparametros[2].append("--CantAnillos");
    listasparametros[2].append(QString::number(Cant_anillos));
    listasparametros[2].append("--Span");
    listasparametros[2].append(QString::number(Span));
    listasparametros[2].append("--MaxDifAnillos");
    listasparametros[2].append(QString::number(Dif_anillos));
    listasparametros[2].append("--CantAngulos");
    listasparametros[2].append(QString::number(cant_ang));
    listasparametros[2].append("--CantRhos");
    listasparametros[2].append(QString::number(cant_rhos));
    listasparametros[2].append("--MaxRho");
    listasparametros[2].append(QString::number(max_Rho));
    listasparametros[2].append("--MaxZ");
    listasparametros[2].append(QString::number(max_Z));

    //  ----- Paso 3, Armo el header del michelograma
    // Armo la linea al programa
    programas[3] = path_INTERFILES+"GenerarEncabezadoInterfile.pl";

    // Armo los argumentos
    listasparametros[3].append("--archivo_datos");
    listasparametros[3].append(QDir::currentPath()+"/"+path_Salida+Nombre_archivo+".i33");
    listasparametros[3].append("--out");
    listasparametros[3].append(QDir::currentPath()+"/"+path_Salida+Nombre_archivo+".h33");
    listasparametros[3].append("--CantAnillos");
    listasparametros[3].append(QString::number(Cant_anillos));
    listasparametros[3].append("--Span");
    listasparametros[3].append(QString::number(Span));
    listasparametros[3].append("--MaxDifAnillos");
    listasparametros[3].append(QString::number(Dif_anillos));
    listasparametros[3].append("--CantAngulos");
    listasparametros[3].append(QString::number(cant_ang));
    listasparametros[3].append("--CantRhos");
    listasparametros[3].append(QString::number(cant_rhos));
    listasparametros[3].append("--MaxRho");
    listasparametros[3].append(QString::number(max_Rho));
    listasparametros[3].append("--MaxZ");
    listasparametros[3].append(QString::number(max_Z));


    // ejecuto el primer proceso de la lista, los sucesivos procesos se ejecutaran en el callback de finalizacion
    proceso->start(programas[indice_ejecucion],listasparametros[indice_ejecucion]);



/*
    QString programa;
    QStringList listaparametros;

    //  ----- Paso 1, parseo

    // Armo la linea al programa
    programa = path_PARSER+"parsear";

    // Armo los argumentos
    listaparametros.append(arch_recon);
    listaparametros.append(QDir::currentPath()+"/"+path_Salida+Nombre_archivo+".dat");


    // ejecuto el proceso
    proceso->start(programa,listaparametros);

    consola->appendPlainText("Proceso de parseado iniciado, no tiene output, tener paciencia...");
    qApp->processEvents();

    // Espero que termine, este programa no tiene salida
    proceso->waitForFinished(-1); // Espero hasta el fin de los tiempos
    proceso->close();
    listaparametros.clear();

    qApp->processEvents();


    //  ----- Paso 2, analizo las coincidencias
    // Armo la linea al programa
    programa = path_INTERFILES+"AnalizoCoincidenciasPET.pl";

    // Armo los argumentos
    listaparametros.append("--Emin");
    listaparametros.append(QString::number(Emin));
    listaparametros.append("--Emax");
    listaparametros.append(QString::number(Emax));
    listaparametros.append("--in");
    listaparametros.append(QDir::currentPath()+"/"+path_Salida+Nombre_archivo+".dat");
    listaparametros.append("--out");
    listaparametros.append(QDir::currentPath()+"/"+path_Salida+Nombre_archivo+".lor");


    cout<<programa.toStdString()<<endl;
    cout<<listaparametros.first().toStdString()<<endl;
    cout<<listaparametros.last().toStdString()<<endl;

    // ejecuto el proceso
    proceso->start(programa,listaparametros);

    consola->appendPlainText("Generando archivo de coincidencia, no tiene output, tener paciencia...");
    qApp->processEvents();

    proceso->waitForFinished(-1);   // Espero hasta el fin de los tiempos
    proceso->close();
    listaparametros.clear();


//    cout<<proceso->errorString().toStdString()<<endl;

*/

    return 1;
}
bool Reconstructor::Reconstruir()
{
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
