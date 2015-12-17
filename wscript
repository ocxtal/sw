#! /usr/bin/env python
# encoding: utf-8

def options(opt):
	opt.load('compiler_c')

def configure(conf):
	conf.load('compiler_c')

	conf.env.append_value('CFLAGS', '-O3')
	conf.env.append_value('CFLAGS', '-std=c99')
	conf.env.append_value('CFLAGS', '-Wall')

def build(bld):
	bld.stlib(source = 'sw.c', target = 'sw')
	bld.shlib(source = 'sw.c', target = 'sw')
	bld.program(source = 'sw.c', target = 'swtest', cflags = '-DTEST')
	bld.install_files('${PREFIX}/include', 'sw.h')
