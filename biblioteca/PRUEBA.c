#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sqlite3.h>

// Prototipos de funciones
void insertarLibro(sqlite3 *db);
void revisarBaseDeDatos(sqlite3 *db);
void tomarPrestamo(sqlite3 *db);
void registrarUsuario(sqlite3 *db);
int verificarUsuario(sqlite3 *db, const char *numero_cuenta);

int main() {
    sqlite3 *db;
    int result = sqlite3_open(":memory:", &db);
    
    if (result != SQLITE_OK) {
        fprintf(stderr, "No se pudo abrir la base de datos: %s\n", sqlite3_errmsg(db));
        return result;
    }
    
    char *error_message = 0;
    
    // Abrir la base de datos o crearla si no existe
      const char *create_usuarios_table = "CREATE TABLE usuarios ("
                                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                        "nombre TEXT,"
                                        "numero_cuenta TEXT,"
                                        "escuela TEXT"
                                        ")";
    
    result = sqlite3_exec(db, create_usuarios_table, 0, 0, &error_message);
    
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error al crear la tabla usuarios: %s\n", error_message);
        sqlite3_free(error_message);
        return result;
    }
    
    // Crear la tabla libros
    const char *create_libros_table = "CREATE TABLE libros ("
                                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                      "nombre TEXT,"
                                      "autor TEXT,"
                                      "tema TEXT,"
                                      "disponibilidad INTEGER,"
                                      "cantidad_disponible INTEGER"
                                      ")";
    
    result = sqlite3_exec(db, create_libros_table, 0, 0, &error_message);
    
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error al crear la tabla libros: %s\n", error_message);
        sqlite3_free(error_message);
        return result;
    }
    
    // Crear la tabla prestamos
    const char *create_prestamos_table = "CREATE TABLE prestamos ("
                                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                         "usuario_id INTEGER,"
                                         "libro_id INTEGER,"
                                         "fecha_prestamo TEXT,"
                                         "FOREIGN KEY(usuario_id) REFERENCES usuarios(id),"
                                         "FOREIGN KEY(libro_id) REFERENCES libros(id)"
                                         ")";
    
    result = sqlite3_exec(db, create_prestamos_table, 0, 0, &error_message);
    
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error al crear la tabla prestamos: %s\n", error_message);
        sqlite3_free(error_message);
        return result;
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
            printf("4. Regresar un préstamo de libro\n");
            printf("5. Salir\n");
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
                    printf("PRUEBA");    
                    break;

                case 5:
                    printf("Saliendo del programa...\n");
                    break;
                    
                default:
                    printf("Opción no válida. Intente nuevamente.\n");
                    break;
            }
            
            printf("\n");
            
        } while (opcion != 4);
    } else {
        printf("Usuario no autorizado. Registrese...\n");
        registrarUsuario(db);
        main();
    }
    
    // Cerrar la base de datos
    sqlite3_close(db);
    
    return 0;
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
    char numero_cuenta[100];
    int libro_id;
    
    printf("\nIngrese su número de cuenta: ");
    scanf(" %[^\n]s", numero_cuenta);
    
    printf("Ingrese el ID del libro que desea tomar en préstamo: ");
    scanf("%d", &libro_id);
    
    if (verificarUsuario(db, numero_cuenta)) {
        // Verificar disponibilidad del libro
        char query_disponibilidad[100];
        sprintf(query_disponibilidad, "SELECT disponibilidad, cantidad_disponible "
                                      "FROM libros WHERE id = %d", libro_id);
        
        sqlite3_stmt *statement_disponibilidad;
        int result_disponibilidad = sqlite3_prepare_v2(db, query_disponibilidad, -1, &statement_disponibilidad, 0);
        
        if (result_disponibilidad == SQLITE_OK) {
            if (sqlite3_step(statement_disponibilidad) == SQLITE_ROW) {
                int disponibilidad = sqlite3_column_int(statement_disponibilidad, 0);
                int cantidad_disponible = sqlite3_column_int(statement_disponibilidad, 1);
                
                if (disponibilidad == 1 && cantidad_disponible > 0) {
                    // Actualizar disponibilidad del libro
                    char query_actualizar_disponibilidad[100];
                    sprintf(query_actualizar_disponibilidad, "UPDATE libros "
                                                             "SET cantidad_disponible = %d "
                                                             "WHERE id = %d",
                                                             cantidad_disponible - 1, libro_id);
                    
                    char *error_message = 0;
                    int result_actualizar_disponibilidad = sqlite3_exec(db, query_actualizar_disponibilidad, 0, 0, &error_message);
                    
                    if (result_actualizar_disponibilidad == SQLITE_OK) {
                        // Registrar préstamo
                        char query_prestamo[200];
                        sprintf(query_prestamo, "INSERT INTO prestamos (usuario_id, libro_id, fecha_prestamo) "
                                                "VALUES ((SELECT id FROM usuarios WHERE numero_cuenta = '%s'), %d, date('now'))",
                                                numero_cuenta, libro_id);
                        
                        int result_prestamo = sqlite3_exec(db, query_prestamo, 0, 0, &error_message);
                        
                        if (result_prestamo == SQLITE_OK) {
                            printf("Préstamo registrado exitosamente.\n");
                        } else {
                            fprintf(stderr, "Error al registrar el préstamo: %s\n", error_message);
                            sqlite3_free(error_message);
                        }
                    } else {
                        fprintf(stderr, "Error al actualizar la disponibilidad del libro: %s\n", error_message);
                        sqlite3_free(error_message);
                    }
                } else {
                    printf("El libro no está disponible en este momento.\n");
                }
            } else {
                printf("No se encontró un libro con el ID especificado.\n");
            }
            
            sqlite3_finalize(statement_disponibilidad);
        } else {
            fprintf(stderr, "Error al verificar la disponibilidad del libro: %s\n", sqlite3_errmsg(db));
        }
    } else {
        printf("El usuario no está registrado en el sistema.\n");
    }
}

void registrarUsuario(sqlite3 *db) {
    char nombre[100];
    char numero_cuenta[100];
    char escuela[100];
    
    printf("\nIngrese su nombre completo: ");
    scanf(" %[^\n]s", nombre);
    
    printf("Ingrese su número de cuenta: ");
    scanf(" %[^\n]s", numero_cuenta);
    
    printf("Ingrese el nombre de su escuela: ");
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
    sprintf(query, "SELECT * FROM usuarios WHERE numero_cuenta = '%s'", numero_cuenta);
    
    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(db, query, -1, &statement, 0);
    
    if (result == SQLITE_OK) {
        if (sqlite3_step(statement) == SQLITE_ROW) {
            sqlite3_finalize(statement);
            return 1;
        } else {
            sqlite3_finalize(statement);
            return 0;
        }
    } else {
        fprintf(stderr, "Error al verificar el usuario: %s\n", sqlite3_errmsg(db));
        return 0;
    }
}
