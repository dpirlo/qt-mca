#include "inc/apExceptions.hpp"
using namespace ap;

/**
 * @brief Exceptions::Exceptions
 *
 * Contructor de la clase que recibe como parámetro un puntero _const char_
 * que es mensaje que emite la excepción.
 *
 * @param desc
 */
Exceptions::Exceptions(const char *desc)
{
  excdesc = desc;
}
