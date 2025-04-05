from testsuite.databases import pgsql


async def test_instance_registration_on_start(pgsql):
    cursor = pgsql['main-db'].cursor()
    cursor.execute('SELECT ip_address, port FROM active_runner_instances')
    instances = cursor.fetchall()
    assert len(instances) == 1
    assert str(instances[0][0]) == 'lambda.miet.ru'
    assert int(instances[0][1]) == 8080
