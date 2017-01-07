def build(bld):
    module = bld.create_ns3_module('ospf', ['core', 'internet'])
    module.includes = '.'
    module.source = [
        'model/ipv6-ospf-routing.cc',
        'model/ospf-lsa-header.cc',
        'model/ospf-link-lsa.cc',
        'model/ospf-router-lsa.cc',
        'model/ospf-header.cc',
        'model/ospf-database-description.cc',
        'model/ospf-link-state-request.cc',
        'model/ospf-hello.cc',
        'model/ospf-routing-table.cc',
    ]

    module_test = bld.create_ns3_module_test_library('ospf')
    module_test.source = [
        # 'test/'
    ]
    
    headers = bld(features='ns3header')
    headers.module = 'ospf'
    headers.source = [
        'model/ipv6-ospf-routing.h',
        'model/ospf-lsa-header.h',
        'model/ospf-link-lsa.h',
        'model/ospf-router-lsa.h',
        'model/ospf-header.h',
        'model/ospf-database-description.h',
        'model/ospf-link-state-request.h',
        'model/ospf-hello.h',
        'model/ospf-routing-table.h',
    ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()
