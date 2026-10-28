from database.base import Base
from pathlib import Path
from sqlalchemy import create_engine, event
from sqlalchemy.engine import URL
from sqlalchemy.orm import sessionmaker

# Sets the relative path to the SQLite database file
# /.../pass-the-controller/database/pass_the_controller.db
db_path = Path(__file__).resolve().parent / "pass_the_controller.db"

# Creates the URL used to create the engine 
# Instead of hard coding the file path, URL.create() allows the OS itself to format it regardless if it's Windows or Linux
db_url = URL.create(drivername = "sqlite", database=str(db_path))


# Create the SQLite database engine according to the database file location
db_engine = create_engine(db_url)

# SQLite by default doesn't enforce foreign key contraints, this function handles that upon creating an engine
@event.listens_for(engine, "connect")
def foreign_keys(dbapi_connection, connection_record)
    cursor = dbapi_connection.cursor()
    cursor.execute("PRAGMA foreign_keys=ON")
    cursor.close()


# Local session details
Session = sessionmaker(
    bind=engine, # Use SQLite
    autoflush=False, # Don't auto flush pending changing
    expire_on_commit=False # Don't refresh upon accessing attributes that are already loaded (If this were true, it would query attributes twice upon entering and exiting the Python-Electron bridge)
)

# Create a new session
def get_session():
    return Session()


# Initialize the database
def init_db():
    Base.metadata.create_all(bind=engine)
