from sqlalchemy.orm import DeclarativeBase


# Shared parent class that registers all SQLAlchemy table models
# Doing this will make all tables be created together during initialization instead of doing it one at a time
class Base(DeclarativeBase):
    pass