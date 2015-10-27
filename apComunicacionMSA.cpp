#include "apComunicacionMSA.hpp"


ComunicacionMSA::ComunicacionMSA()
{

}

void ComunicacionMSA::Abort()
{
    Cancelar = true;
}

void ComunicacionMSA::Init(string port)
{
    bool ret = true;
    pmtCount = 0;
    InitVars();

    SerialPortMCA = new SerialPort(port);
    SerialPortMCA.ReadTimeout = 2000;
    SerialPortMCA.WriteTimeout = 2000;
    SerialPortMCA.BaudRate = 921600;

    try
    {
      if (!OpenPort())
         {
         SetError("No se puede abrir el puerto en Init()");
         ret = false;
          }
          else
          {
          ///Comando de Inicialización
          ///Se envía el comando de incialización: "@0064010;\r"
          ///y se debe recibir la respuesta "@0064020;\r"
          SerialPortMCA.DiscardInBuffer();
          SerialPortMCA.NewLine = Convert.ToString('\r'); //Defino el Enter para fin de recepcion
          SerialPortMCA.ReadExisting();
          SerialPortMCA.WriteLine("@0064010;");

          string strTramaRecepcion;
          ///Utilizando el metodo ReadLine() para recibir los datos, se finaliza
          ///la recepción con el caracter de retorno de carro '\r'.
          ///El mismo es quitado del string de recepción de manera autómatica,
          ///por lo que no hay que tenerlo en cuenta.
          strTramaRecepcion = SerialPortMCA.ReadLine();

          if (U.Left(strTramaRecepcion, 5) == "@0064")//&& U.Right(strTramaRecepcion, 2) == "0<")
             {
              pmtCount = Convert.ToInt16(U.HexRawToDec(U.Mid(strTramaRecepcion, 6, 2))) - 1;
              InitVars();
              }
         else
             {
              SetError("Respuesta inesperada: " + strTramaRecepcion + " en Init()");
              ret = false;
             }
         }
        ClosePort();
        return ret;
     }
     catch (TimeoutException ToutExc)
     {
           SetError(ToutExc.Message + " en Init()");
           SerialPortMCA.DiscardInBuffer();
           return false;
     }
     catch (Exception exc)
     {
           SetError("La Placa MCA no responde en Init() : " + exc.Message);
           return false;
     }

}

bool ComunicacionMSA::InitVars()
{
     CantCanales = 1024;
     SetMinMaxVentana(50, CantCanales - 1);
     SetCanales(CantCanales);

     pmtHV = new int[pmtCount + 1];
     pmtTemp = new int[pmtCount + 1];
     pmtTarget = new float[pmtCount + 1];
     pmtPick = new int[pmtCount + 1];
     Multicanal = new UInt32[pmtCount + 1][];
     LastCanal = new UInt32[CantCanales];

     for (int i = 0; i <= pmtCount; i++)
          Multicanal[i] = new UInt32[CantCanales];

     return true;
}
