import argparse
import tensorflow as tf
from pathlib import Path
tf.get_logger().setLevel('ERROR') # shhhhh

import sys

from .dataset import DataSet, BackgroundMode
from .trainer import train, train_with_images
from .tabulate import tabulate
from .plotter import plot


def parse_arguments(args) -> argparse.Namespace:
    parser = argparse.ArgumentParser(prog='ml_tool')
    subparsers = parser.add_subparsers(help='This tool has several modes.', dest="subtool")

    train_parser = subparsers.add_parser("train", help="Train ML models")
    train_parser.add_argument("-d", "--data-directory", type=str, default="data", help="Where to load data files from")
    train_parser.add_argument("-m", "--model-directory", type=str, default="models", help="Where to store model files")
    train_parser.add_argument("-l", "--leave-one-out", action="store_true", default=False, help="Run in leave-one-out mode for feature selection.")
    train_parser.add_argument("-i", "--images", action="store_true", default=False, help="Run with convolutional images.")

    group = train_parser.add_mutually_exclusive_group()
    group.add_argument("--train-qq", action='store_true', help="Train on qq only")
    group.add_argument("--train-gg", action='store_true', help="Train on gg only")
    group2 = train_parser.add_mutually_exclusive_group()
    group2.add_argument("--test-qq", action='store_true', help="Test on qq only")
    group2.add_argument("--test-gg", action='store_true', help="Test on gg only")

    train_parser.add_argument("name", type=str, help="Model name")

    plot_parser = subparsers.add_parser("plot", help="Plot ML models")
    plot_parser.add_argument("-d", "--data-directory", type=str, default="data", help="Where to load data files from")
    plot_parser.add_argument("-m", "--model-directory", type=str, default="models", help="Where to load model files")
    plot_parser.add_argument("-p", "--plot-directory", type=str, default="plots", help="Where to store plot images")
    plot_parser.add_argument("-i", "--images", action="store_true", default=False, help="Run with convolutional images.")

    group2 = plot_parser.add_mutually_exclusive_group()
    group2.add_argument("--test-qq", action='store_true', help="Test on qq only")
    group2.add_argument("--test-gg", action='store_true', help="Test on gg only")

    tabulate_parser = subparsers.add_parser("tabulate", help="Tabulate ML models")
    tabulate_parser.add_argument("-m", "--model-directory", type=str, default="models", help="Where to load model files")
    tabulate_parser.add_argument("variable", type=str, help="Variable name")

    return parser, parser.parse_args(args)


def command(args):
    parser, arguments = parse_arguments(args)
    if not arguments:
        parser.print_help()
        return 1

    if arguments.subtool == 'train':
        train_mode = BackgroundMode.Mixed
        if arguments.train_qq:
            train_mode = BackgroundMode.QQOnly
        elif arguments.train_gg:
            train_mode = BackgroundMode.GGOnly

        test_mode = BackgroundMode.Mixed
        if arguments.test_qq:
            test_mode = BackgroundMode.QQOnly
        elif arguments.test_gg:
            test_mode = BackgroundMode.GGOnly

        keys = DataSet.nominal_keys.copy()
        if arguments.images:
            keys.append('jet_image')

        dataset = DataSet(arguments.data_directory, train_mode, test_mode, keys)

        if arguments.images:
            train_with_images(dataset, arguments.model_directory, arguments.name)
        else:
            train(dataset, arguments.model_directory, arguments.name, arguments.leave_one_out)
    if arguments.subtool == 'tabulate':
        tabulate(arguments.model_directory, arguments.variable)
    if arguments.subtool == 'plot':
        train_mode = BackgroundMode.Mixed
        test_mode = BackgroundMode.Mixed
        if arguments.test_qq:
            test_mode = BackgroundMode.QQOnly
        elif arguments.test_gg:
            test_mode = BackgroundMode.GGOnly

        keys = DataSet.nominal_keys.copy()
        if arguments.images:
            keys.append('jet_image')

        dataset = DataSet(arguments.data_directory, train_mode, test_mode, keys)
        modeldir = Path(arguments.model_directory).resolve()
        plotdir = Path(arguments.plot_directory).resolve()
        plot(modeldir, plotdir, dataset)


def main():
    return command(sys.argv[1:])


if __name__ == "__main__":
    main()
