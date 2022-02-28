import sys
import traceback
import argparse
from datetime import datetime
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
    compariplot.add_argument("-f", "--filename", type=str, default="", help="output plot filename")
    compariplot.add_argument("-s", "--markersize", type=float, default=3, help="markersize")

    ##tabulate
    tabulate_parser = subparsers.add_parser("tabulate", help="Tabulate ML models")
    tabulate_parser.add_argument("-m", "--model-directory", type=str, default="models", help="Where to load model files")
    tabulate_parser.add_argument("variable", type=str, help="Variable name")

    ##correlate
    correlate_parser = subparsers.add_parser("correlate", help="Correlate 2 ML models")
    correlate_parser.add_argument("-d", "--data-directory", type=str, default="data", help="Where to load data files from")
    correlate_parser.add_argument("-m1", "--model1", type=str, help="Model 1")
    correlate_parser.add_argument("-m2", "--model2", type=str, help="Model 2")

    ##reevaluate
    reevaluate_parser = subparsers.add_parser("reevaluate", help="Re-evaluate ML models")
    reevaluate_parser.add_argument("-d", "--data-directory", type=str, default="data", help="Where to load data files from")
    reevaluate_parser.add_argument("-m", "--model", type=str, help="Model")

    group3 = reevaluate_parser.add_mutually_exclusive_group()
    group3.add_argument("--train-qq", action='store_true', help="Train on qq only")
    group3.add_argument("--train-gg", action='store_true', help="Train on gg only")

    group4 = reevaluate_parser.add_mutually_exclusive_group()
    group4.add_argument("--test-qq", action='store_true', help="Test on qq only")
    group4.add_argument("--test-gg", action='store_true', help="Test on gg only")

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
        import tensorflow as tf
        gpus = tf.config.experimental.list_physical_devices('GPU')
        tf.config.experimental.set_memory_growth(gpus[0], True)

        dataset = DataSet(arguments.data_directory, BackgroundMode.Mixed, BackgroundMode.Mixed)

        start = datetime.now()
        num_runs = sum(1 for x in get_configs(arguments.config_file))

        ##get config file:
        i = 0
        iterable = iter(get_configs(arguments.config_file))
        config = next(iterable)
        while True:
            try:
                while True:
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

                    if config['run_options'] == 'conv_only' or config['run_options'] == 'combi':
                        keys.append('jet_image')

                    dataset.train_mode = train_mode
                    dataset.test_mode = test_mode
                    dataset.reset_keys(keys)
                    try:
                        train(dataset, arguments.model_directory, config)
                    except KeyboardInterrupt as e:
                        raise e
                    except:
                        with open(f"{arguments.model_directory}/{i}.log", "w") as f:
                            f.write(traceback.format_exc())
                        print(f"Model {i} failed to train, exception logged to file {arguments.model_directory}/{i}.log")

                    # Time projection
                    now = datetime.now()
                    duration = now - start
                    total_duration = duration / (i + 1) * num_runs
                    left_duration = total_duration - duration
                    finished = now + left_duration
                    print(f"{i+1}/{num_runs} done, time elapsed: {duration}, estimated time left: {left_duration}, projected finish by {finished}. ")
                    i += 1
                    config = next(iterable)
            except KeyboardInterrupt:
                del dataset
                del train
                import gc
                gc.collect()
                tf.keras.backend.clear_session()
                print("Pausing, do you wish to continue? [y/n].")
                pauset = datetime.now()
                while True:
                    a = input(':')
                    if a == 'n':
                        sys.exit(0)
                    if a == 'y':
                        break
                from .trainer import train
                dataset = DataSet(arguments.data_directory, BackgroundMode.Mixed, BackgroundMode.Mixed)
                start -= datetime.now() - pauset

    ## make table
    if arguments.subtool == 'tabulate':
        from .tabulate import tabulate
        tabulate(arguments.model_directory, arguments.variable)

    ## correlate
    if arguments.subtool == 'correlate':
        from .correlate import correlate
        dataset = DataSet(arguments.data_directory, BackgroundMode.Mixed, BackgroundMode.Mixed)
        correlate(Path(arguments.model1), Path(arguments.model2), dataset)

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
            arguments.color_category,
            arguments.filename,
            arguments.markersize
        )
    ##reevaluate models
    if arguments.subtool == 'reevaluate':
        from .reevaluate import reevaluate
        import tensorflow as tf

        gpus = tf.config.experimental.list_physical_devices('GPU')
        tf.config.experimental.set_memory_growth(gpus[0], True)

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

        dataset = DataSet(arguments.data_directory, train_mode, test_mode)
        reevaluate(Path(arguments.model), dataset)


def main():
    return command(sys.argv[1:])


if __name__ == "__main__":
    main()
