#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sqlite3.h>
#include <time.h>

// Prototipos de funciones
void insertarLibro(sqlite3 *db);
void revisarBaseDeDatos(sqlite3 *db);
void tomarPrestamo(sqlite3 *db);
void devolverLibro(sqlite3 *db);
void registrarUsuario(sqlite3 *db);
void registrarPrestamo(sqlite3 *db, int libro_id); 
void revisarPrestamos(sqlite3 *db);
void buscarlibrotema(sqlite3 *db);
void buscarlibrotitulo(sqlite3 *db);
int verificarUsuario(sqlite3 *db, const char *numero_cuenta);


int main() {
    sqlite3 *db;
    char *error_message = 0;
    
    // Abrir la base de datos o crearla si no existe
    int result = sqlite3_open("biblioteca.db", &db);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error al abrir/crear la base de datos: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    
    // Crear la tabla "libros" si no existe
    const char *create_table_query = "CREATE TABLE IF NOT EXISTS libros("
                                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                     "nombre TEXT NOT NULL,"
                                     "autor TEXT NOT NULL,"
                                     "tema TEXT NOT NULL,"
                                     "disponibilidad INTEGER NOT NULL,"
                                     "cantidad_disponible INTEGER NOT NULL)";
    result = sqlite3_exec(db, create_table_query, 0, 0, &error_message);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error al crear la tabla de libros: %s\n", error_message);
        sqlite3_free(error_message);
        sqlite3_close(db);
        return 1;
    }
    
    // Crear la tabla "usuarios" si no existe
    const char *create_table_usuarios_query = "CREATE TABLE IF NOT EXISTS usuarios("
                                              "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                              "nombre TEXT NOT NULL,"
                                              "numero_cuenta TEXT NOT NULL UNIQUE,"
                                              "escuela TEXT NOT NULL)";
    result = sqlite3_exec(db, create_table_usuarios_query, 0, 0, &error_message);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error al crear la tabla de usuarios: %s\n", error_message);
        sqlite3_free(error_message);
        sqlite3_close(db);
        return 1;
    }
    
 // Crear la tabla "prestamos" si no existe
const char *create_table_prestamos_query = "CREATE TABLE IF NOT EXISTS prestamos("
                                           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                           "numero_cuenta INTEGER NOT NULL,"
                                           "libro_id INTEGER NOT NULL,"
                                           "fecha_prestamo TEXT NOT NULL,"
                                           "fecha_devolucion TEXT NOT NULL,"
                                           "estado TEXT NOT NULL,"
                                           "FOREIGN KEY(numero_cuenta) REFERENCES usuarios(numero_cuenta),"
                                           "FOREIGN KEY(libro_id) REFERENCES libros(id))";
result = sqlite3_exec(db, create_table_prestamos_query, 0, 0, &error_message);
if (result != SQLITE_OK) {
    fprintf(stderr, "Error al crear la tabla de préstamos: %s\n", error_message);
    sqlite3_free(error_message);
    sqlite3_close(db);
    return 1;
}

    
    // Menú de opciones
    int opcion;
    char numero_cuenta[100];
    printf("Ingrese su número de cuenta: ");
    scanf("%s", numero_cuenta);
    
    // Verificar si el usuario existe y tiene acceso permitido
    if (verificarUsuario(db, numero_cuenta)) {
        do {
            printf("\nSeleccione una opción:\n");
            printf("1. Insertar un libro\n");
            printf("2. Revisar la base de datos de libros\n");
            printf("3. Tomar un préstamo de libro\n");
            printf("4. Devolver un libro\n");
            printf("5. Revisar prestamos\n");
            printf("6. Buscar libro por tema. \n");
            printf("7. Buscar libro por titulo. \n");
            printf("8. Salir\n");
            printf("Opción: ");
            scanf("%d", &opcion);
            
            switch (opcion) {
                case 1:
                    insertarLibro(db);
                    break;
                    
                case 2:
                    revisarBaseDeDatos(db);
                    break;
                    
                case 3:
                    tomarPrestamo(db);
                    break;
                
                case 4: 
                    devolverLibro(db);
                    break;
                
                case 5:
                    revisarPrestamos (db) ;
                    break;
                
                case 6:
                    buscarlibrotema (db) ;
                    break;

                case 7:
                    buscarlibrotitulo (db);
                    break;

                case 8:
                    printf("Saliendo del programa...\n");
                    break;
                    
                default:
                    printf("Opción no válida. Intente nuevamente.\n");
                    break;
            }
            
            printf("\n");
            
        } while (opcion != 8);
    } else {
        printf("Usuario no autorizado. Regístrese...\n");
        registrarUsuario(db);
        main();
    }
    
    // Cerrar la base de datos
    sqlite3_close(db);
    
    return 0;
}

void revisarPrestamos(sqlite3 *db) {
    char *query = "SELECT id, numero_cuenta, libro_id, fecha_prestamo, fecha_devolucion, estado FROM prestamos";
    sqlite3_stmt *statement;
    
    int result = sqlite3_prepare_v2(db, query, -1, &statement, 0);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    printf("Tabla de préstamos:\n");
    printf("ID  |  Número de cuenta  |  Libro ID  |  Fecha de préstamo  |  Fecha de devolución  |  Estado\n");
    printf("---------------------------------------------------------------------------------------------\n");
    
    while (sqlite3_step(statement) == SQLITE_ROW) {
        int id = sqlite3_column_int(statement, 0);
        int numero_cuenta = sqlite3_column_int(statement, 1);
        int libro_id = sqlite3_column_int(statement, 2);
        const unsigned char *fecha_prestamo = sqlite3_column_text(statement, 3);
        const unsigned char *fecha_devolucion = sqlite3_column_text(statement, 4);
        const unsigned char *estado = sqlite3_column_text(statement, 5);
        
        printf("%-4d|  %-17d|  %-9d|  %-19s|  %-19s|  %-19s\n", id, numero_cuenta, libro_id, fecha_prestamo, fecha_devolucion, estado);
    }
    
    sqlite3_finalize(statement);
}




    void buscarlibrotema(sqlite3 *db) {
    char tema[100];
    
    printf("\nIngrese el tema del libro que desea buscar: ");
    scanf(" %[^\n]s", tema);
    
    char query[200];
    sprintf(query, "SELECT * FROM libros WHERE tema LIKE '%%%s%%'", tema);
    
    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(db, query, -1, &statement, 0);
    
    if (result == SQLITE_OK) {
        printf("\nResultados de búsqueda:\n\n");
        printf("ID   | Nombre            | Autor             | Tema                | Disponibilidad   | Cantidad Disponible\n");
        printf("-----|-------------------|-------------------|---------------------|------------------|-------------------\n");
        
        while (sqlite3_step(statement) == SQLITE_ROW) {
            int id = sqlite3_column_int(statement, 0);
            char *nombre = (char *)sqlite3_column_text(statement, 1);
            char *autor = (char *)sqlite3_column_text(statement, 2);
            char *tema = (char *)sqlite3_column_text(statement, 3);
            int disponibilidad = sqlite3_column_int(statement, 4);
            int cantidad_disponible = sqlite3_column_int(statement, 5);
            
            printf("%-5d| %-18s| %-18s| %-21s| %-16d| %-19d\n", id, nombre, autor, tema, disponibilidad, cantidad_disponible);
        }
        
        sqlite3_finalize(statement);
    } else {
        fprintf(stderr, "Error al realizar la búsqueda: %s\n", sqlite3_errmsg(db));
    }
}
    void buscarlibrotitulo(sqlite3 *db) {
    char nombre[100];
    
    printf("\nIngrese el nombre del libro que desea buscar: ");
    scanf(" %[^\n]s", nombre);
    
    char query[200];
    sprintf(query, "SELECT * FROM libros WHERE nombre LIKE '%%%s%%'", nombre);
    
    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(db, query, -1, &statement, 0);
    
    if (result == SQLITE_OK) {
        printf("\nResultados de búsqueda:\n\n");
        printf("ID   | Nombre            | Autor             | Tema                | Disponibilidad   | Cantidad Disponible\n");
        printf("-----|-------------------|-------------------|---------------------|------------------|-------------------\n");
        
        while (sqlite3_step(statement) == SQLITE_ROW) {
            int id = sqlite3_column_int(statement, 0);
            char *nombre = (char *)sqlite3_column_text(statement, 1);
            char *autor = (char *)sqlite3_column_text(statement, 2);
            char *tema = (char *)sqlite3_column_text(statement, 3);
            int disponibilidad = sqlite3_column_int(statement, 4);
            int cantidad_disponible = sqlite3_column_int(statement, 5);
            
            printf("%-5d| %-18s| %-18s| %-21s| %-16d| %-19d\n", id, nombre, autor, tema, disponibilidad, cantidad_disponible);
        }
        
        sqlite3_finalize(statement);
    } else {
        fprintf(stderr, "Error al realizar la búsqueda: %s\n", sqlite3_errmsg(db));
    }
}


void insertarLibro(sqlite3 *db) {
    char nombre[100];
    char autor[100];
    char tema[100];
    int disponibilidad;
    int cantidad_disponible;
    
    printf("\nIngrese el nombre del libro: ");
    scanf(" %[^\n]s", nombre);
    
    printf("Ingrese el autor del libro: ");
    scanf(" %[^\n]s", autor);
    
    printf("Ingrese el tema del libro: ");
    scanf(" %[^\n]s", tema);
    
    printf("Ingrese la disponibilidad del libro (1 = Disponible, 0 = No disponible): ");
    scanf("%d", &disponibilidad);
    
    printf("Ingrese la cantidad disponible del libro: ");
    scanf("%d", &cantidad_disponible);
    
    char query[200];
    sprintf(query, "INSERT INTO libros (nombre, autor, tema, disponibilidad, cantidad_disponible) "
                   "VALUES ('%s', '%s', '%s', %d, %d)",
                   nombre, autor, tema, disponibilidad, cantidad_disponible);
    
    char *error_message = 0;
    int result = sqlite3_exec(db, query, 0, 0, &error_message);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error al insertar el libro: %s\n", error_message);
        sqlite3_free(error_message);
    } else {
        printf("Libro insertado exitosamente.\n");
    }
}

void revisarBaseDeDatos(sqlite3 *db) {
    printf("\nTabla de libros:\n\n");
    printf("ID   | Nombre            | Autor             | Tema                | Disponibilidad   | Cantidad Disponible\n");
    printf("-----|-------------------|-------------------|---------------------|------------------|-------------------\n");
    
    char *query = "SELECT * FROM libros";
    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(db, query, -1, &statement, 0);
    
    if (result == SQLITE_OK) {
        while (sqlite3_step(statement) == SQLITE_ROW) {
            int id = sqlite3_column_int(statement, 0);
            char *nombre = (char *)sqlite3_column_text(statement, 1);
            char *autor = (char *)sqlite3_column_text(statement, 2);
            char *tema = (char *)sqlite3_column_text(statement, 3);
            int disponibilidad = sqlite3_column_int(statement, 4);
            int cantidad_disponible = sqlite3_column_int(statement, 5);
            
            printf("%-5d| %-18s| %-18s| %-21s| %-16d| %-19d\n", id, nombre, autor, tema, disponibilidad, cantidad_disponible);
        }
        
        sqlite3_finalize(statement);
    } else {
        fprintf(stderr, "Error al leer la tabla de libros: %s\n", sqlite3_errmsg(db));
    }
}



void tomarPrestamo(sqlite3 *db) {
    int libro_id;
    printf("\nIngrese el ID del libro que desea tomar en préstamo: ");
    scanf("%d", &libro_id);
    
    char query[100];
    sprintf(query, "SELECT * FROM libros WHERE id = %d", libro_id);
    
    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(db, query, -1, &statement, 0);
    
    if (result == SQLITE_OK) {
        if (sqlite3_step(statement) == SQLITE_ROW) {
            int disponibilidad = sqlite3_column_int(statement, 4);
            int cantidad_disponible = sqlite3_column_int(statement, 5);
            
            if (disponibilidad == 1 && cantidad_disponible > 0) {
                // Actualizar la disponibilidad del libro y la cantidad disponible
                char update_query[100];
                sprintf(update_query, "UPDATE libros SET disponibilidad = 1, cantidad_disponible = %d WHERE id = %d",
                        cantidad_disponible - 1, libro_id);
                
                char *error_message = 0;
                int update_result = sqlite3_exec(db, update_query, 0, 0, &error_message);
                if (update_result != SQLITE_OK) {
                    fprintf(stderr, "Error al tomar el préstamo del libro: %s\n", error_message);
                    sqlite3_free(error_message);
                } else {
                    printf("Libro prestado exitosamente.\n");
                    
                    // Registrar el préstamo en la tabla de préstamos
                    registrarPrestamo(db, libro_id);
                }
            } 
            else             
                if (disponibilidad == 1 && cantidad_disponible == 1) {
                // Actualizar la disponibilidad del libro y la cantidad disponible
                char update_query[100];
                sprintf(update_query, "UPDATE libros SET disponibilidad = 0, cantidad_disponible = %d WHERE id = %d",
                        cantidad_disponible - 1, libro_id);
                
                char *error_message = 0;
                int update_result = sqlite3_exec(db, update_query, 0, 0, &error_message);
                if (update_result != SQLITE_OK) {
                    fprintf(stderr, "Error al tomar el préstamo del libro: %s\n", error_message);
                    sqlite3_free(error_message);
                } else {
                    printf("Libro prestado exitosamente.\n");
                    
                    // Registrar el préstamo en la tabla de préstamos
                    registrarPrestamo(db, libro_id);
                }
            } 
            else  {
                printf("El libro no está disponible en este momento.\n");
            }
        } else {
            printf("No se encontró ningún libro con el ID especificado.\n");
        }
        
        sqlite3_finalize(statement);
    } else {
        fprintf(stderr, "Error al consultar el libro: %s\n", sqlite3_errmsg(db));
    }
}

void registrarPrestamo(sqlite3 *db, int libro_id) {
    printf("\nIngrese el ID del libro que desea tomar: ");
    scanf("%d", &libro_id);

    int numero_cuenta;
    printf("Ingrese el número de cuenta del usuario: ");
    scanf("%d", &numero_cuenta);

    // Obtener la fecha actual
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char fecha_prestamo[11];
    strftime(fecha_prestamo, sizeof(fecha_prestamo), "%Y-%m-%d", timeinfo);

    // Calcular la fecha de devolución
    struct tm *devolucion_timeinfo = localtime(&rawtime);
    devolucion_timeinfo->tm_mday += 14;  // Agregar 14 días a la fecha actual
    mktime(devolucion_timeinfo);  // Normalizar la fecha
    char fecha_devolucion[11];
    strftime(fecha_devolucion, sizeof(fecha_devolucion), "%Y-%m-%d", devolucion_timeinfo);

    char estado[] = "Prestado";  // Utilizar un array de caracteres para almacenar la cadena

    // Realizar la inserción del préstamo en la tabla de préstamos
    char insert_query[200];
    sprintf(insert_query, "INSERT INTO prestamos (numero_cuenta, libro_id, fecha_prestamo, fecha_devolucion, estado) "
                      "VALUES (%d, %d, '%s', '%s', '%s')", numero_cuenta, libro_id, fecha_prestamo, fecha_devolucion, estado);

    char *error_message = 0;
    int result = sqlite3_exec(db, insert_query, 0, 0, &error_message);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error al registrar el préstamo: %s\n", error_message);
        sqlite3_free(error_message);
    } else {
        printf("Préstamo registrado exitosamente.\n");
    }
}


void devolverLibro(sqlite3 *db) {
    int libro_id;
    printf("\nIngrese el ID del libro que desea devolver: ");
    scanf("%d", &libro_id);

    int numero_cuenta;
    printf("Ingrese su número de cuenta: ");
    scanf("%d", &numero_cuenta);

    // Consultar la información del préstamo del libro para el usuario dado
    char query[200];
    sprintf(query, "SELECT * FROM prestamos WHERE libro_id = %d AND numero_cuenta IN (SELECT numero_cuenta FROM usuarios WHERE numero_cuenta = %d)", libro_id, numero_cuenta);

    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(db, query, -1, &statement, 0);

    if (result == SQLITE_OK) {
        if (sqlite3_step(statement) == SQLITE_ROW) {
            int prestamo_id = sqlite3_column_int(statement, 0);

            // Actualizar la disponibilidad del libro en la tabla libros
            char update_query[200];
            sprintf(update_query, "UPDATE libros SET disponibilidad = 1, cantidad_disponible = cantidad_disponible + 1 WHERE id = %d",
                    libro_id);

            // Actualizar la devolución del libro en la tabla prestamos
            char estado[] = "Devuelto";  // Utilizar un array de caracteres para almacenar la cadena
            char update_prestamo_query[200];
            sprintf(update_prestamo_query, "UPDATE prestamos SET estado = '%s', fecha_devolucion = DATE('now') WHERE id = %d",
                   estado, prestamo_id);  // Agregar una coma entre estado y fecha_devolucion

            char *error_message = 0;

            // Iniciar una transacción para asegurar la atomicidad de las actualizaciones
            sqlite3_exec(db, "BEGIN TRANSACTION", 0, 0, &error_message);

            int update_result = sqlite3_exec(db, update_query, 0, 0, &error_message);
            if (update_result != SQLITE_OK) {
                fprintf(stderr, "Error al actualizar la disponibilidad del libro: %s\n", error_message);
                sqlite3_free(error_message);
                sqlite3_exec(db, "ROLLBACK", 0, 0, &error_message);
            } else {
                update_result = sqlite3_exec(db, update_prestamo_query, 0, 0, &error_message);
                if (update_result != SQLITE_OK) {
                    fprintf(stderr, "Error al actualizar la devolución del libro: %s\n", error_message);
                    sqlite3_free(error_message);
                    sqlite3_exec(db, "ROLLBACK", 0, 0, &error_message);
                } else {
                    printf("Libro devuelto exitosamente.\n");
                    sqlite3_exec(db, "COMMIT", 0, 0, &error_message);
                }
            }
        } else {
            printf("No se encontró ningún préstamo para el libro con el ID especificado y el número de cuenta ingresado.\n");
        }

        sqlite3_finalize(statement);
    } else {
        fprintf(stderr, "Error al consultar el préstamo del libro: %s\n", sqlite3_errmsg(db));
    }
}




void registrarUsuario(sqlite3 *db) {
    char nombre[100];
    char numero_cuenta[100];
    char escuela[100];
    
    printf("Ingrese su nombre: ");
    scanf(" %[^\n]s", nombre);
    
    printf("Ingrese su número de cuenta: ");
    scanf(" %[^\n]s", numero_cuenta);
    
    printf("Ingrese su escuela: ");
    scanf(" %[^\n]s", escuela);
    
    char query[200];
    sprintf(query, "INSERT INTO usuarios (nombre, numero_cuenta, escuela) "
                   "VALUES ('%s', '%s', '%s')",
                   nombre, numero_cuenta, escuela);
    
    char *error_message = 0;
    int result = sqlite3_exec(db, query, 0, 0, &error_message);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error al registrar el usuario: %s\n", error_message);
        sqlite3_free(error_message);
    } else {
        printf("Usuario registrado exitosamente.\n");
    }
}

int verificarUsuario(sqlite3 *db, const char *numero_cuenta) {
    char query[200];
    sprintf(query, "SELECT id FROM usuarios WHERE numero_cuenta = '%s'", numero_cuenta);
    
    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(db, query, -1, &statement, 0);
    
    if (result == SQLITE_OK) {
        if (sqlite3_step(statement) == SQLITE_ROW) {
            int user_id = sqlite3_column_int(statement, 0);
            sqlite3_finalize(statement);
            return user_id;
        }
        
        sqlite3_finalize(statement);
    }
    
    return 0;
}
