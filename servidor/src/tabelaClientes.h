#ifndef TABELA_CLIENTES_H
#define TABELA_CLIENTES_H

#include <stdint.h>

int processar_transacao(uint32_t ip_origem, uint32_t id_req, uint32_t valor);
void* obter_cliente(uint32_t address);

#endif
