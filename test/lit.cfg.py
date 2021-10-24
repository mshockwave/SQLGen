import lit.formats

config.name = 'SQLGen'
config.test_format = lit.formats.ShTest(True)

config.suffixes = ['.td']
config.excludes = ['Table.td']

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.obj_root, 'test')

config.substitutions.append(('sqlgen',
    os.path.join(config.obj_root, 'sqlgen -I=' + config.test_source_root)))
config.substitutions.append(('%FileCheck', config.filecheck_path))
