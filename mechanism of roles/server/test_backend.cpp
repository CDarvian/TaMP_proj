#include <QTest>
#include <QCoreApplication>
#include <QSqlDatabase>
#include "functions_to_server.h"
#include "databasemanager.h"

class TestBackend : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();     // Выполняется один раз перед всеми тестами
    void cleanupTestCase(); // Выполняется один раз после всех тестов

    // Тестовые функции
    void testSHA512();
    void testDivision();
    void testGraph();
    void testDatabase();
};

void TestBackend::initTestCase()
{
    QCoreApplication::setOrganizationName("TestOrg");
    QCoreApplication::setApplicationName("BackendTest");

    // Проверяем доступность драйвера SQLite
    QVERIFY2(QSqlDatabase::isDriverAvailable("QSQLITE"),
             "SQLite driver not available");

    // Инициализируем тестовую базу данных
    QString dbPath = "Test.db";
    QVERIFY2(DatabaseManager::instance().init(dbPath),
             "Failed to initialize database");

    // Проверяем существование таблицы User
    QString checkTable = DatabaseManager::instance().execute(
        "SELECT name FROM sqlite_master WHERE type='table' AND name='User';");
    QVERIFY2(!checkTable.startsWith("ERROR:") && !checkTable.isEmpty(),
             "User table not found or database error");

    // Очищаем тестовые данные
    DatabaseManager::instance().execute("DELETE FROM User;");
}

void TestBackend::cleanupTestCase()
{
    // Очищаем тестовые данные после всех тестов
    DatabaseManager::instance().execute("DELETE FROM User;");
}

void TestBackend::testSHA512()
{
    // Тестируем хеширование SHA-512
    QString input = "password123";
    QString expected = "bed4efa1d4fdbd954bd3705d6a2a78270ec9a52ecfbfb010c61862af5c76af1761ffeb1aef6aca1bf5d02b3781aa854fabd2b69c790de74e17ecfec3cb6ac4bf";

    QString result = handleSHA512(input);
    qDebug() << "SHA512:" << result;
    QVERIFY2(result.contains(expected),
             qPrintable(QString("Expected: %1\nActual: %2").arg(expected).arg(result)));

    // Проверка на пустой ввод
    result = handleSHA512("");
    QVERIFY2(result.startsWith("SHA512_ERR"), qPrintable(result));
}

void TestBackend::testDivision()
{
    // Проверяем вычисление квадратного корня
    QString result = handleDivision("16.0");
    QVERIFY2(result.startsWith("DIV_RESULT: 4"), qPrintable(result));

    // Проверка на неверный формат
    result = handleDivision("abc");
    QVERIFY2(result.startsWith("DIV_ERR"), qPrintable(result));
}

void TestBackend::testGraph()
{
    // Проверяем работу с графом
    QString result = handleShortest("1,4");
    QVERIFY2(result.startsWith("GRAPH: 1"), qPrintable(result)); // В тестовом графе путь 1-4

    result = handleShortest("1,3");
    QVERIFY2(result.startsWith("GRAPH: 2"), qPrintable(result)); // Путь 1-2-3

    // Проверка на неверный формат
    result = handleShortest("a,b");
    QVERIFY2(result.startsWith("GRAPH_ERR"), qPrintable(result));
}

void TestBackend::testDatabase()
{
    // Проверяем обработку неверного SQL
    QString result = DatabaseManager::instance().execute("INVALID SQL");
    QVERIFY2(result.startsWith("ERROR:"), "Should return error for invalid SQL");

    // Проверяем вставку и выборку данных
    QString sql = "INSERT INTO User(login,password,status) VALUES('testuser','testpass','offline');";
    result = DatabaseManager::instance().execute(sql);
    QVERIFY2(result.startsWith("OK:"), qPrintable(result));

    sql = "SELECT login FROM User WHERE login='testuser';";
    result = DatabaseManager::instance().execute(sql);
    QVERIFY2(result.contains("testuser"), qPrintable(result));
}



QTEST_MAIN(TestBackend)
#include "test_backend.moc"
