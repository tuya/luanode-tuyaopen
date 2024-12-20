# -*- coding: utf-8 -*-
# Copyright (C) 2015-2019 Peter Magnusson <peter@kmpm.se>

"""This module is the cli for the Uploader class"""

from __future__ import print_function

import argparse
import logging
import os
import sys
import glob
import serial
from .uploader import Uploader
from .term import terminal
from serial import VERSION as serialversion
from .version import __version__


log = logging.getLogger(__name__)  # pylint: disable=C0103


def destination_from_source(sources, use_glob=True):
    """
    Split each of the sources in the array on ':'
    First part will be source, second will be destination.
    Modifies the the original array to contain only sources
    and returns an array of destinations.
    """
    destinations = []
    newsources = []
    for i in range(0, len(sources)):
        srcdst = sources[i].split(':')
        if len(srcdst) == 2:
            destinations.append(srcdst[1])
            newsources.append(srcdst[0])  # proper list assignment
        else:
            if use_glob:
                listing = glob.glob(srcdst[0])
                for filename in listing:
                    newsources.append(filename)
                    # always use forward slash at destination
                    destinations.append(filename.replace('\\', '/'))
            else:
                newsources.append(srcdst[0])
                destinations.append(srcdst[0])

    return [newsources, destinations]



def operation_upload(uploader, sources, verify, do_compile, do_file, do_restart):
    """The upload operation"""
    if not isinstance(sources, list):
        sources = [sources]
    sources, destinations = destination_from_source(sources)
    if len(destinations) == len(sources):
        # if uploader.prepare():
        for filename, dst in zip(sources, destinations):
            if do_compile:
                uploader.file_remove(os.path.splitext(dst)[0]+'.lc')
            if not os.path.exists(filename) and not os.path.isfile(filename):
                raise Exception("File does not exist. {filename}".format(filename=filename))
            uploader.write_file(filename, dst, verify)
            # init.lua is not allowed to be compiled
            if do_compile and dst != 'init.lua':
                uploader.file_compile(dst)
                uploader.file_remove(dst)
                if do_file:
                    uploader.file_do(os.path.splitext(dst)[0]+'.lc')
            elif do_file:
                uploader.file_do(dst)
        # else:
        #     raise Exception('Error preparing nodemcu for reception')
    else:
        raise Exception('You must specify a destination filename for each file you want to upload.')

    if do_restart:
        uploader.node_restart()
    log.info('All done!')
    return destinations


def operation_download(uploader, sources, *args, **kwargs):
    """The download operation"""
    sources, destinations = destination_from_source(sources, False)
    # print('sources', sources)
    # print('destinations', destinations)
    dest = kwargs.pop('dest', '')
    if len(destinations) == len(sources):
        if uploader.prepare():
            for filename, dst in zip(sources, destinations):
                dst = os.path.join(dest, dst)
                uploader.read_file(filename, dst)
    else:
        raise Exception('You must specify a destination filename for each file you want to download.')
    log.info('All done!')


def operation_list_files(uploader):
    """List file on target"""
    files = uploader.file_list()
    for f in files:
        log.info("{file:30s} {size}".format(file=f[0], size=f[1]))


def operation_file(uploader, cmd, filename=''):
    """File operations"""
    if cmd == 'list':
        operation_list_files(uploader)
    if cmd == 'do':
        for path in filename:
            uploader.file_do(path)
    elif cmd == 'format':
        uploader.file_format()
    elif cmd == 'remove':
        for path in filename:
            uploader.file_remove(path)
    elif cmd == 'print':
        for path in filename:
            uploader.file_print(path)
    elif cmd == 'remove_all':
        uploader.file_remove_all()


def operation_port(args):
    if args.cmd == 'list':
        ports = serial.tools.list_ports.comports(include_links=False)
        print('device', 'vid', 'pid')
        for p in ports:
            print(p.device, p.vid, p.pid)


def arg_auto_int(value):
    """parsing function for integer arguments"""
    return int(value, 0)


def main_func():
    """Main function for cli"""
    parser = argparse.ArgumentParser(
        description='NodeMCU Lua file uploader',
        prog='nodemcu-uploader'
        )

    parser.add_argument(
        '--verbose',
        help='verbose output',
        action='store_true',
        default=False)

    parser.add_argument(
        '--silent',
        help='silent output. Errors and worse',
        action='store_true',
        default=False)

    parser.add_argument(
        '--version',
        help='prints the version and exists',
        action='version',
        version='%(prog)s {version} (serial {serialversion}, python {pv})'.format(
            version=__version__,
            serialversion=serialversion,
            pv=sys.version)
    )

    parser.add_argument(
        '--port', '-p',
        help='Serial port device',
        default=Uploader.PORT)

    parser.add_argument(
        '--baud', '-b',
        help='Serial port baudrate',
        type=arg_auto_int,
        default=Uploader.BAUD)

    parser.add_argument(
        '--start_baud', '-B',
        help='Initial Serial port baudrate',
        type=arg_auto_int,
        default=Uploader.START_BAUD)

    parser.add_argument(
        '--timeout', '-t',
        help='Timeout for operations',
        type=arg_auto_int,
        default=Uploader.TIMEOUT)

    parser.add_argument(
        '--autobaud_time', '-a',
        help='Duration of the autobaud timer',
        type=float,
        default=Uploader.AUTOBAUD_TIME,
    )

    subparsers = parser.add_subparsers(
        dest='operation',
        help='Run nodemcu-uploader {command} -h for additional help')

    backup_parser = subparsers.add_parser(
        'backup',
        help='Backup all the files on the nodemcu board')
    backup_parser.add_argument('path', help='Folder where to store the backup')

    upload_parser = subparsers.add_parser(
        'upload',
        help='Path to one or more files to be uploaded. Destination name will be the same as the file name.')

    upload_parser.add_argument(
        'filename',
        nargs='+',
        help='Lua file to upload. Use colon to give alternate destination.'
        )

    upload_parser.add_argument(
        '--compile', '-c',
        help='If file should be uploaded as compiled',
        action='store_true',
        default=False
        )

    upload_parser.add_argument(
        '--verify', '-v',
        help='To verify the uploaded data.',
        action='store',
        nargs='?',
        choices=['none', 'raw', 'sha1'],
        default='none'
        )

    upload_parser.add_argument(
        '--dofile', '-e',
        help='If file should be run after upload.',
        action='store_true',
        default=False
        )

    upload_parser.add_argument(
        '--restart', '-r',
        help='If esp should be restarted',
        action='store_true',
        default=False
    )

    exec_parser = subparsers.add_parser(
        'exec',
        help='Path to one or more files to be executed line by line.')

    exec_parser.add_argument('filename', nargs='+', help='Lua file to execute.')

    download_parser = subparsers.add_parser(
        'download',
        help='Path to one or more files to be downloaded. Destination name will be the same as the file name.')

    download_parser.add_argument(
        'filename',
        nargs='+',
        help='Lua file to download. Use colon to give alternate destination.')

    file_parser = subparsers.add_parser(
        'file',
        help='File functions')

    file_parser.add_argument(
        'cmd',
        choices=('list', 'do', 'format', 'remove', 'print', 'remove_all'),
        help="""list=list files, do=dofile given path, format=formate file area,
            remove=remove given path, remove_all=delete all files""")

    file_parser.add_argument('filename', nargs='*', help='path for cmd')

    node_parse = subparsers.add_parser(
        'sys',
        help='Node functions')

    node_parse.add_argument(
        'ncmd',
        choices=('heap', 'restart', 'info'),
        help="heap=print heap memory, restart=restart nodemcu, info=show node info")

    subparsers.add_parser(
        'terminal',
        help='Run pySerials miniterm'
    )

    port_parser = subparsers.add_parser(
        'port',
        help='serial port stuff'
    )

    port_parser.add_argument(
        'cmd',
        choices=('list',)
    )

    args = parser.parse_args()

    default_level = logging.INFO
    if args.silent:
        default_level = logging.ERROR
    if args.verbose:
        default_level = logging.DEBUG

    # formatter = logging.Formatter('%(message)s')

    logging.basicConfig(level=default_level, format='%(message)s')

    if args.operation == 'terminal':
        # uploader can not claim the port
        terminal(args.port, str(args.start_baud))
        return
    elif args.operation == 'port':
        operation_port(args)
        return

    # let uploader user the default (short) timeout for establishing connection
    uploader = Uploader(args.port, args.baud, start_baud=args.start_baud, autobaud_time=args.autobaud_time)

    # and reset the timeout (if we have the uploader&timeout)
    if args.timeout:
        uploader.set_timeout(args.timeout)

    if args.operation == 'upload':
        operation_upload(uploader, args.filename, args.verify, args.compile, args.dofile,
                         args.restart)

    elif args.operation == 'download':
        operation_download(uploader, args.filename)

    elif args.operation == 'exec':
        sources = args.filename
        for path in sources:
            uploader.exec_file(path)

    elif args.operation == 'file':
        operation_file(uploader, args.cmd, args.filename)

    elif args.operation == 'sys':
        if args.ncmd == 'heap':
            uploader.node_heap()
        elif args.ncmd == 'restart':
            uploader.node_restart()
        elif args.ncmd == 'info':
            uploader.node_info()

    elif args.operation == 'backup':
        uploader.backup(args.path)


    # no uploader related commands after this point
    uploader.close()
