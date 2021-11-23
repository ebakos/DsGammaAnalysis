import sys
import argparse
from pathlib import Path

from .dataset import DataSet, BackgroundMode


def parse_arguments(args) -> argparse.Namespace:
    parser = argparse.ArgumentParser(prog='ml_tool')
    subparsers = parser.add_subparsers(help='This tool has several modes.', dest="subtool")

    train_parser = subparsers.add_parser("train", help="Train ML models")
    train_parser.add_argument("-d", "--data-directory", type=str, default="data", help="Where to load data files from")
    train_parser.add_argument("-m", "--model-directory", type=str, default="models", help="Where to store model files")
    train_parser.add_argument("-j", "--config-file", type=str, default="default_model.json", help="json file with config options")

    ##plot...
    plot_parser = subparsers.add_parser("plot", help="Plot ML models")
    plot_parser.add_argument("-d", "--data-directory", type=str, default="data", help="Where to load data files from")
    plot_parser.add_argument("-m", "--model-directory", type=str, default="models", help="Where to load model files")
    plot_parser.add_argument("-p", "--plot-directory", type=str, default="plots", help="Where to store plot images")
    plot_parser.add_argument("-i", "--images", action="store_true", default=False, help="Run with convolutional images.")

    group2 = plot_parser.add_mutually_exclusive_group()
    group2.add_argument("--test-qq", action='store_true', help="Test on qq only")
    group2.add_argument("--test-gg", action='store_true', help="Test on gg only")

    ##compariplot...
    compariplot = subparsers.add_parser("compariplot", help="Make an overview of models as a seaborn swarmplot")
    compariplot.add_argument("-m", "--model-directory", type=str, default="models", help="Where to load model files")
    compariplot.add_argument("-p", "--plot-directory", type=str, default="plots", help="Where to store plot images")
    compariplot.add_argument("-r", "--range", type=float, nargs=2, default=None, help="Y-axis range")
    compariplot.add_argument("-c", "--constraint", action='append', type=str, nargs=2, help="constraints on variables")
    compariplot.add_argument("category", type=str, help="Category for the X axis")
    compariplot.add_argument("variable", type=str, help="Variable out of metadata which to put on the Y axis")
    compariplot.add_argument("-o", "--color-category", type=str, help="colour of points category")

    ##tabulate
    tabulate_parser = subparsers.add_parser("tabulate", help="Tabulate ML models")
    tabulate_parser.add_argument("-m", "--model-directory", type=str, default="models", help="Where to load model files")
    tabulate_parser.add_argument("variable", type=str, help="Variable name")

    return parser, parser.parse_args(args)


def command(args):
    parser, arguments = parse_arguments(args)
    if not arguments:
        parser.print_help()
        return 1

    ##train models
    if arguments.subtool == 'train':
        from .trainer import train
        from .config import get_configs

        ##get config file:
        for config in get_configs(arguments.config_file):
            train_mode = BackgroundMode.Mixed
            if config['train_qq']:
                train_mode = BackgroundMode.QQOnly
            elif config['train_gg']:
                train_mode = BackgroundMode.GGOnly

            test_mode = BackgroundMode.Mixed
            if config['test_qq']:
                test_mode = BackgroundMode.QQOnly
            elif config['test_gg']:
                test_mode = BackgroundMode.GGOnly

            keys = DataSet.nominal_keys.copy()

            if config['run_options'] == 'conv_only' or config['run_options'] == 'dense_conv':
                keys.append('jet_image')

            dataset = DataSet(arguments.data_directory, train_mode, test_mode, keys)
            train(dataset, arguments.model_directory, config)

    ## make table
    if arguments.subtool == 'tabulate':
        from .tabulate import tabulate
        tabulate(arguments.model_directory, arguments.variable)

    ##plot models
    if arguments.subtool == 'plot':
        from .plotter import plot
        train_mode = BackgroundMode.Mixed
        test_mode = BackgroundMode.Mixed
        if arguments.test_qq:
            test_mode = BackgroundMode.QQOnly
        elif arguments.test_gg:
            test_mode = BackgroundMode.GGOnly

        dataset = DataSet(arguments.data_directory, train_mode, test_mode)
        modeldir = Path(arguments.model_directory).resolve()
        plotdir = Path(arguments.plot_directory).resolve()
        plot(modeldir, plotdir, dataset)

    ##compariplot models
    if arguments.subtool == "compariplot":
        from .compariplot import compariplot
        compariplot(
            arguments.model_directory,
            arguments.plot_directory,
            arguments.range,
            arguments.constraint,
            arguments.category,
            arguments.variable,
            arguments.color_category
        )


def main():
    return command(sys.argv[1:])


if __name__ == "__main__":
    main()
