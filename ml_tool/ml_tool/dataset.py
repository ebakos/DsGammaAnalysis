from enum import Enum, auto
from pathlib import Path

from sklearn.model_selection import train_test_split
from sklearn.utils import shuffle
import numpy as np
import uproot



class BackgroundMode(Enum):
    Mixed = auto()
    QQOnly = auto()
    GGOnly = auto()


class DataSet:
    nominal_keys = [
        'delta_eta', 'delta_phi', 'n_neutral', 'n_charged', 'charge', 'invariant_mass', 'btag',
        'e_had_over_e_em', 'tau_0', 'tau_1', 'tau_2', 'abs_qj', 'r_em', 'r_track', 'f_em', 'p_core_1',
        'p_core_2', 'f_core_1', 'f_core_2', 'f_core_3', 'pt_d_square', 'les_houches_angularity', 'width',
        'mass', 'track_magnitude'
    ]

    @staticmethod
    def transform(keys, data):
        keys = keys.copy()
        if 'jet_image' in keys:
            jet_img_data = data['jet_image'].array(library='np')[:80000]
            keys.remove('jet_image')
            if not keys:
                return jet_img_data

            otherdata = np.array(data.arrays(keys, library='np', how=tuple)).T[:80000]
            return otherdata, jet_img_data
        return np.array(data.arrays(keys, library='np', how=tuple)).T[:80000]

    def __init__(self, directory, train_mode=BackgroundMode.Mixed, test_mode=BackgroundMode.Mixed, keys=nominal_keys):
        path = Path(directory).resolve()
        self.gg_up = uproot.open(str(path / 'gg_ntuples.root') + ":DS")
        self.qq_up = uproot.open(str(path / 'qq_ntuples.root') + ":DS")
        self.wp_up = uproot.open(str(path / 'wp_ntuples.root') + ":DS")
        self.wm_up = uproot.open(str(path / 'wm_ntuples.root') + ":DS")
        self.train_mode = train_mode
        self.test_mode = test_mode
        self.reset_keys(keys)

    def _preload_data(self):
        self.gg_data = self.transform(self._keys, self.gg_up)
        self.qq_data = self.transform(self._keys, self.qq_up)
        self.wp_data = self.transform(self._keys, self.wp_up)
        self.wm_data = self.transform(self._keys, self.wm_up)

        (self.gg_data_train, self.gg_data_test), (self.qq_data_train, self.qq_data_test), \
        (self.wp_data_train, self.wp_data_test), (self.wm_data_train, self.wm_data_test) = (\
            train_test_split(
                ds,
                test_size=0.3,
                shuffle=True,
                random_state=0
            )
            for ds in (self.gg_data, self.qq_data, self.wp_data, self.wm_data)
        )

    def _preload_data_with_image(self):
        self.gg_data, self.gg_images = self.transform(self._keys, self.gg_up)
        self.qq_data, self.qq_images = self.transform(self._keys, self.qq_up)
        self.wp_data, self.wp_images = self.transform(self._keys, self.wp_up)
        self.wm_data, self.wm_images = self.transform(self._keys, self.wm_up)

        (self.gg_data_train, self.gg_data_test, self.gg_image_train, self.gg_image_test), \
        (self.qq_data_train, self.qq_data_test, self.qq_image_train, self.qq_image_test), \
        (self.wp_data_train, self.wp_data_test, self.wp_image_train, self.wp_image_test), \
        (self.wm_data_train, self.wm_data_test, self.wm_image_train, self.wm_image_test) = (\
            train_test_split(
                ds, img,
                test_size=0.3,
                shuffle=True,
                random_state=0
            )
            for ds, img in (
                (self.gg_data, self.gg_images),
                (self.qq_data, self.qq_images),
                (self.wp_data, self.wp_images),
                (self.wm_data, self.wm_images)
            )
        )

    def reset_keys(self, keys):
        self._keys = keys.copy()
        if 'jet_image' in self._keys and len(self._keys) > 1:
            self._preload_data_with_image()
        else:
            self._preload_data()

    def keys(self):
        return self._keys

    def image_dimensions(self):
        return self.qq_images[0].shape

    def _dataset(self, bkgs, sigs):
        data = np.concatenate(list(bkgs) + list(sigs), axis=0)
        labels =np.concatenate((
                np.zeros((sum((len(bkg) for bkg in bkgs))),),
                np.ones((sum((len(sig) for sig in sigs))),),
            ), axis=0)
        return shuffle(data, labels, random_state=0)

    def _dataset_withimage(self, bkgs, image_bkgs, sigs, image_sigs):
        data = np.concatenate(list(bkgs) + list(sigs), axis=0)
        images = np.concatenate(list(image_bkgs) + list(image_sigs), axis=0)
        labels =np.concatenate((
                np.zeros((sum((len(bkg) for bkg in bkgs))),),
                np.ones((sum((len(sig) for sig in sigs))),),
            ), axis=0)
        ds_d, ds_i, ds_l = shuffle(data, images, labels, random_state=0)
        return [ds_d, ds_i], ds_l

    def train_data(self):
        if 'jet_image' in self._keys and len(self._keys) > 1:
            if self.train_mode == BackgroundMode.Mixed:
                return self._dataset_withimage(
                    bkgs=[self.gg_data_train, self.qq_data_train],
                    image_bkgs=[self.gg_image_train, self.qq_image_train],
                    sigs=[self.wp_data_train, self.wm_data_train],
                    image_sigs=[self.wp_image_train, self.wm_image_train]
                )
            elif self.train_mode == BackgroundMode.GGOnly:
                return self._dataset_withimage(
                    bkgs=[self.gg_data_train],
                    image_bkgs=[self.gg_image_train],
                    sigs=[self.wp_data_train, self.wm_data_train],
                    image_sigs=[self.wp_image_train, self.wm_image_train]
                )
            elif self.train_mode == BackgroundMode.QQOnly:
                return self._dataset_withimage(
                    bkgs=[self.qq_data_train],
                    image_bkgs=[self.qq_image_train],
                    sigs=[self.wp_data_train, self.wm_data_train],
                    image_sigs=[self.wp_image_train, self.wm_image_train]
                )
        if self.train_mode == BackgroundMode.Mixed:
            return self._dataset(
                bkgs=[self.gg_data_train, self.qq_data_train],
                sigs=[self.wp_data_train, self.wm_data_train]
            )
        elif self.train_mode == BackgroundMode.GGOnly:
            return self._dataset(
                bkgs=[self.gg_data_train],
                sigs=[self.wp_data_train, self.wm_data_train]
            )
        elif self.train_mode == BackgroundMode.QQOnly:
            return self._dataset(
                bkgs=[self.self.qq_data_train],
                sigs=[self.wp_data_train, self.wm_data_train]
            )
        else:
            raise Exception("Invalid train mode")

    def test_data(self):
        if 'jet_image' in self._keys and len(self._keys) > 1:
            if self.test_mode == BackgroundMode.Mixed:
                return self._dataset_withimage(
                    bkgs=[self.gg_data_test, self.qq_data_test],
                    image_bkgs=[self.gg_image_test, self.qq_image_test],
                    sigs=[self.wp_data_test, self.wm_data_test],
                    image_sigs=[self.wp_image_test, self.wm_image_test]
                )
            elif self.test_mode == BackgroundMode.GGOnly:
                return self._dataset_withimage(
                    bkgs=[self.gg_data_test],
                    image_bkgs=[self.gg_image_test],
                    sigs=[self.wp_data_test, self.wm_data_test],
                    image_sigs=[self.wp_image_test, self.wm_image_test]
                )
            elif self.test_mode == BackgroundMode.QQOnly:
                return self._dataset_withimage(
                    bkgs=[self.qq_data_test],
                    image_bkgs=[self.qq_image_test],
                    sigs=[self.wp_data_test, self.wm_data_test],
                    image_sigs=[self.wp_image_test, self.wm_image_test]
                )
        if self.test_mode == BackgroundMode.Mixed:
            return self._dataset(
                bkgs=[self.gg_data_test, self.qq_data_test],
                sigs=[self.wp_data_test, self.wm_data_test]
            )
        elif self.test_mode == BackgroundMode.GGOnly:
            return self._dataset(
                bkgs=[self.gg_data_test],
                sigs=[self.wp_data_test, self.wm_data_test]
            )
        elif self.test_mode == BackgroundMode.QQOnly:
            return self._dataset(
                bkgs=[self.self.qq_data_test],
                sigs=[self.wp_data_test, self.wm_data_test]
            )
        else:
            raise Exception("Invalid test mode")
