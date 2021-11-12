from dataclasses import dataclass
from typing import Dict, List, Union
from pathlib import Path
import json

from tensorflow import keras


@dataclass
class Model:
    model: keras.Model
    name: str
    metadata: Dict[str, Union[str, int, bool]]

    def save(self, directory) -> None:
        self.model.save(str(Path(directory).resolve() / self.name))
        (Path(directory).resolve() / self.name / ".custom.metadata.json").write_text(json.dumps({
            "name": self.name,
            "metadata": self.metadata
        }))

    @classmethod
    def load(cls, file) -> 'Model':
        return cls(
            model=keras.models.load_model(str(file)),
            **json.loads((file / ".custom.metadata.json").read_text())
        )

    @classmethod
    def load_multi(cls, directory) -> List['Model']:
        return [cls.load(file) for file in Path(directory).resolve().glob("*")]


