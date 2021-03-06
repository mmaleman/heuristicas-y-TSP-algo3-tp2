#include "heuristicas.h"
#include <random>
#include<algorithm>

int obtenerElVecinoNoVisitadoMasCercano (int actual, Grafo &g, vector<bool> &visitados) {
    int n = g.size();
    int elMasCercano = -1;

    for(int i = 0; i < n; i++){
        if(
            !visitados[i] && //no se puede volver a visitar nodos
            (elMasCercano == -1 || g[actual][i] < g[actual][elMasCercano]) //que le gane a elMasCercano
            ) {
            elMasCercano = i;
        }
    }

    return elMasCercano;
}


// Heurística constructiva golosa con la idea de agregar en cada paso al vecino más cercano al último nodo agregado.
Hamiltoniano heuristicaVecinoMasCercano(Grafo &g, int nodoInicial) {
    int n = g.size();
    vector<bool> visitados(n, false);
    Hamiltoniano circuitoHamiltoniano;

	circuitoHamiltoniano.push_back(nodoInicial);
    int actual = nodoInicial;
    visitados[nodoInicial] = true;

    while(!todosVisitados(visitados)) {
        int elMasCercano = obtenerElVecinoNoVisitadoMasCercano(actual, g, visitados);

        circuitoHamiltoniano.push_back(elMasCercano);

        actual = elMasCercano;
        visitados[actual] = true;
    }

    return circuitoHamiltoniano;
}


/*************************************************************************************************************/


// Heurística constructiva golosa con la idea de calcular el AGM, recorrerlo mediante DFS y conectar los nodos según el orden dado por DFS.
Hamiltoniano heuristicaAGM(Grafo &g) {
    int n = g.size();
    Grafo t = AGM(g);
    Hamiltoniano ordDFS = DFS(t,0);

    return ordDFS;
}


/*************************************************************************************************************/


int elegirNodo(Grafo &g, vector<bool> &visitados, vector<int> &insertados) {
	// Como criterio de elección tomaremos el vértice más cercano a un vértice que ya está en el circuito. Sino dsp vemos otro criterio
	int n = g.size();
	int elegido = -1;
	int distanciaAlElegido = INFINITO;
	int distanciaAlActual = INFINITO;

	for (int i = 0; i < insertados.size(); i++) {
		int masCercanoAlActual = obtenerElVecinoNoVisitadoMasCercano(i, g, visitados);
		distanciaAlActual = g[i][masCercanoAlActual];
		if(distanciaAlActual < distanciaAlElegido) {
			elegido = masCercanoAlActual;
			distanciaAlElegido = distanciaAlActual;
		}
	}

	return elegido;
}


void insertarElementoEnPosicion(vector<int> &v, int valor, int indice) {
	v.push_back(valor);

	for(int i = v.size() - 1; i > indice; i--) {
		//swappeo i con i-1
		int aux = v[i-1];
		v[i-1] = v[i];
		v[i] = aux;
	}
}


void insertarNodo(int elegido, Grafo &g, vector<bool> &visitados, vector<int> &insertados) {
	// Como criterio de inserción tomaremos dos nodos consecutivos i e i+1 tales que minimicen el costo de insertar al nuevo nodo entre i e i+1.
	int costoResultante = INFINITO;
	int costoActual;
	int izquierda;
	int derecha;

	for (int i = 0; i < insertados.size() - 1; i++) {
		// Si tengo (v1) -- (v2) -- (v3) y estoy parado en (v2) necesito saber hacia dónde avanzar, sino toy yendo para atras y me pierdo para siempre (?).
		costoActual = g[insertados[i]][elegido] + g[elegido][insertados[i+1]] - g[insertados[i]][insertados[i+1]];
		if(costoActual < costoResultante) {
				costoResultante = costoActual;
				izquierda = i;
				derecha = i+1;
		}
	}

	// me fijo a mano el costo de insertarlo entre el nodo final y el inicial del ciclo porque sino el for iba a ser asqueroso (?)
	costoActual =  g[insertados[insertados.size()-1]][insertados[0]];
	if(costoActual < costoResultante) {
		costoResultante = costoActual;
		izquierda = insertados.size()-1;
		derecha = 0;
	}

	// queremos meter izquierda - elegido - derecha
	insertarElementoEnPosicion(insertados, elegido, derecha);
}


// Heurística constructiva golosa con la idea de formar un ciclo inicial y en cada paso agregar un nodo al ciclo que minimice el costo.
Hamiltoniano heuristicaDeInsercion(Grafo &g) {
	int n = g.size();
	Grafo circuitoHamiltoniano(n, vector<Peso>(n, -1));
	vector<bool> visitados(n, false);
	Hamiltoniano insertados; // vector en el que el nodo i es adyacente a los nodos i-1 e i+1. Capaz ya podemos volar el vector visitados pero por ahora lo dejo.

	//elegimos 3 nodos cualesquiera para formar un ciclo inicial. En particular, tomamos los primeros 3 nodos:
	visitados[0] = true;
	visitados[1] = true;
	visitados[2] = true;
	insertados.push_back(0);
	insertados.push_back(1);
	insertados.push_back(2);

	while(!todosVisitados(visitados)) { // Iteramos hasta que estén todos los nodos insertados en el ciclo, es decir, hasta que estén todos visitados.
		int elegido = elegirNodo(g, visitados, insertados);
		visitados[elegido] = true;
		insertarNodo(elegido, g, visitados, insertados);
	}

	return insertados;
}


/*************************************************************************************************************/


Hamiltoniano dosOpt(Hamiltoniano ciclo, int i, int j) {
	std::reverse(ciclo.begin() + i, ciclo.begin() + j + 1);

    return ciclo;
}


vector<Hamiltoniano> obtenerSubVecindad(Hamiltoniano solucionParcial, Grafo &g, float probabilidadDeDescarte) {
	vector<Hamiltoniano> vecindad;
	int n = g.size();
	default_random_engine generator (42);
	bernoulli_distribution distribution(probabilidadDeDescarte);

	for (int i = 0; i < n; i++) {
		for (int j = i+1; j < n; j++) {
			if(!distribution(generator)) {
				Hamiltoniano sol2opt = dosOpt(solucionParcial, i, j);
				vecindad.push_back(sol2opt);
			}
		}
	}

	return vecindad;
}


Hamiltoniano obtenerMejor(vector<Hamiltoniano> &soluciones, Grafo &g) {
	Hamiltoniano res;
	if(soluciones.size() > 0) {
		int elMejor = 0;
		for (int i = 1; i < soluciones.size(); i++){	
			if(costo(soluciones[i], g) < costo(soluciones[elMejor], g)) {
				elMejor = i;
			}
		}
		res = soluciones[elMejor];
	} else {
		res = Hamiltoniano(0, 0);
	}

	return res;
}


Hamiltoniano obtenerMejorConMemoriaDeSoluciones(vector<Hamiltoniano> &vecinos, vector<Hamiltoniano> &memoria, Grafo &g) {
	vector<Hamiltoniano> vecinosFiltrados;
	//saco de los posibles vecinos, aquellos que esten en memoria
	for (int j = 0; j < vecinos.size(); j++){
        bool agrego = true;
		for (int i = 0; i < memoria.size(); i++){
            if(memoria[i].size() != 0) {
			    agrego = agrego && !sonIguales(vecinos[j], memoria[i]);
            }
		}
        if (agrego) vecinosFiltrados.push_back(vecinos[j]);
	}

	return obtenerMejor(vecinosFiltrados, g);
}

// esta la puedo volar chami?
Hamiltoniano solucionHardcoded(Grafo &g) {
	Hamiltoniano res;

	for(int i = 0;i<g.size();i++) {
		res.push_back(i);
	}

	return res;
}


// Metaheurística tabú cuya memoria guarda las últimas soluciones exploradas.
Hamiltoniano heuristicaTabuSolucionesExploradas(Grafo &g, Hamiltoniano solucionInicial(Grafo&), string criterioDeParada,int umbral, int tamanioMemoria, float probabilidadDeDescarte ) {
	Hamiltoniano ciclo = solucionInicial(g);
	Hamiltoniano mejor = ciclo;
	vector<Hamiltoniano> memoria(tamanioMemoria,vector<int>{});
	int indiceMasViejoDeLaMemoria = 0;
	int cantIteracionesSinMejora = 0;
	int cantIteraciones = 0;
	int* criterio;

	if (criterioDeParada == "cantIteracionesSinMejora") {
		criterio = &cantIteracionesSinMejora;
	} else {
		criterio = &cantIteraciones;
	}

	while ( *criterio < umbral ){
		vector<Hamiltoniano> vecinos = obtenerSubVecindad(ciclo, g, probabilidadDeDescarte);
		Hamiltoniano nuevo = obtenerMejorConMemoriaDeSoluciones(vecinos, memoria, g);
		if(nuevo.size() > 0) {
			ciclo = nuevo;
			memoria[indiceMasViejoDeLaMemoria] = ciclo;
			indiceMasViejoDeLaMemoria = (indiceMasViejoDeLaMemoria + 1) % tamanioMemoria;
		}
		if(costo(ciclo, g) < costo(mejor, g)) {
			mejor = ciclo;
		} else {
			cantIteracionesSinMejora++;
		}
		cantIteraciones++;
	}

	return mejor;
}


/*************************************************************************************************************/


pair<Hamiltoniano, pair<int,int>> obtenerMejorConInfo(vector<pair<Hamiltoniano, pair<int,int>>> &soluciones, Grafo &g) {
	pair<Hamiltoniano, pair<int,int>> res;
	if(soluciones.size() > 0) {
		int elMejor = 0;

		for (int i = 1; i < soluciones.size(); i++) {
			if(costo(soluciones[i].first, g) < costo(soluciones[elMejor].first, g)) {
				elMejor = i;
			}
		}
		res = soluciones[elMejor];
	} else {
		res = make_pair(Hamiltoniano(0, 0), make_pair(-1,-1));
	}
	return res;
}


vector<pair<Hamiltoniano, pair<int,int>>> obtenerSubVecindadConInfoIntercambios(Hamiltoniano solucionParcial, Grafo &g, float probabilidadDeDescarte) {
	vector<pair<Hamiltoniano, pair<int,int>>> vecindad;
	int n = g.size();
	default_random_engine generator (42);
	bernoulli_distribution distribution(probabilidadDeDescarte);

	for (int i = 0; i < n; i++) {
		for (int j = i+1; j < n; j++) {
			if(!distribution(generator)) {
            	Hamiltoniano sol2opt = dosOpt(solucionParcial, i, j);
				pair<int,int> intercambio (i,j);
				vecindad.push_back(pair<Hamiltoniano, pair<int,int>>(sol2opt, intercambio));
			}
		}
	}

	return vecindad;
}


pair<Hamiltoniano, pair<int,int>> obtenerMejorConMemoriaDeAristasConAspiracion(vector<pair<Hamiltoniano, pair<int,int>>> &vecinos , vector<pair<int,int>> &memoria, Grafo &g, int mejorCostoHastaAhora) {
	vector<pair<Hamiltoniano, pair<int,int>>> vecinosFiltrados;

	//saco de los posibles vecinos, aquellos que esten en memoria
	for (int j = 0; j < vecinos.size(); j++){
        bool agrego = true;
		if(!(costo(vecinos[j].first, g) < mejorCostoHastaAhora)) {
			for (int i = 0; i < memoria.size(); i++){
				if(memoria[i].first != -1) {
					agrego = agrego && (memoria[i].first != vecinos[j].second.first || memoria[i].second != vecinos[j].second.second);
				}

			}
		}
        if (agrego) vecinosFiltrados.push_back(vecinos[j]);
	}

	return obtenerMejorConInfo(vecinosFiltrados, g);
}

pair<Hamiltoniano, pair<int,int>> obtenerMejorConMemoriaDeAristas(vector<pair<Hamiltoniano, pair<int,int>>> &vecinos , vector<pair<int,int>> &memoria, Grafo &g) {
	vector<pair<Hamiltoniano, pair<int,int>>> vecinosFiltrados;

	//saco de los posibles vecinos, aquellos que esten en memoria
	for (int j = 0; j < vecinos.size(); j++){
        bool agrego = true;
		for (int i = 0; i < memoria.size(); i++){
			if(memoria[i].first != -1) {
				agrego = agrego && (memoria[i].first != vecinos[j].second.first || memoria[i].second != vecinos[j].second.second);
			}

		}
        if (agrego) vecinosFiltrados.push_back(vecinos[j]);
	}

	return obtenerMejorConInfo(vecinosFiltrados, g);
}


// Metaheurística tabú cuya memoria guarda los últimos swaps entre pares de aristas.
Hamiltoniano heuristicaTabuAristasIntercambiadas(Grafo &g, Hamiltoniano solucionInicial(Grafo&), string criterioDeParada,int umbral, int tamanioMemoria, float probabilidadDeDescarte) {
	pair<Hamiltoniano, pair<int,int>> ciclo = make_pair(solucionInicial(g),make_pair(-1,-1));
	pair<Hamiltoniano, pair<int,int>> mejor = ciclo;
	int costoMejor = costo(mejor.first, g);
	vector<pair<int, int>> memoria(tamanioMemoria, pair<int,int>(-1,-1));
	int indiceMasViejoDeLaMemoria = 0;
	int cantIteracionesSinMejora = 0;
	int cantIteraciones = 0;
	int* criterio;

	if (criterioDeParada == "cantIteracionesSinMejora") {
		criterio = &cantIteracionesSinMejora;
	} else {
		criterio = &cantIteraciones;
	}

	while (*criterio < umbral) {
		vector<pair<Hamiltoniano, pair<int,int>>> vecinos = obtenerSubVecindadConInfoIntercambios(ciclo.first, g, probabilidadDeDescarte);
		pair<Hamiltoniano, pair<int,int>> nuevo = obtenerMejorConMemoriaDeAristas(vecinos, memoria, g);
		
		if(nuevo.first.size() > 0) {
			ciclo = nuevo;
			memoria[indiceMasViejoDeLaMemoria] = ciclo.second;
			indiceMasViejoDeLaMemoria = (indiceMasViejoDeLaMemoria + 1) % tamanioMemoria;
		}
		if(costo(ciclo.first, g) < costoMejor) {
			mejor = ciclo;
			costoMejor = costo(mejor.first, g);
		} else {
			cantIteracionesSinMejora++;
		}
		cantIteraciones++;
	}

	return mejor.first;
}
