#!/usr/bin/perl
##############################################################
#  Script        : makeDoxy.pl
#  Autor         : Ariel Hernández (ahestevenz)
#  Fecha         : 12/01/2017
#  Última edición: 12/01/2017, ahestevenz
##############################################################
# Descripción:
#  Agrega la última versión de la aplicación qt-mca y genera
#  la documentación en Doxygen.
# Nota:
# - Se elimina el archivo generado por el script
##############################################################
use strict;
use warnings;

# Archivos de configuración de Doxygen
my $doxy_pre = 'Doxyfile';
my $doxy_use = 'Doxy_version';

# Versión de la aplicación qt-mca
my $version = `git describe --long`;
print "La versión actual de la aplicación qt-mca es: $version";
$version =~ s/\R//g; # Se elimina el carriage return de la variable

# Se abren los archivos y se agrega la versión actual del software
open(my $doxyin, '<', $doxy_pre)
  or die "No se puede abrir el archivo de configuración de Doxygen: '$doxy_pre' $!";
open(my $doxyout,'>', $doxy_use)
  or die "No se puede abrir el archivo de configuración de Doxygen: '$doxy_use' $!";

  while(my $line = <$doxyin> ) {
      $line =~ s/GITVERSION/$version/g;
      print $doxyout $line;
  }
close($doxyin);
close($doxyout);

# Ejecutamos Doxygen con el archivo de configuración modificado
`doxygen $doxy_use`;

# Eliminamos el archivo de configuración generado
unlink $doxy_use;

