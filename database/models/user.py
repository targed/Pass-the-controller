from sqlalchemy import String, LargeBinary, CheckConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base


# User table
class User(Base):
    __tablename__ = "USER"

    user_id: Mapped[str] = mapped_column(
        String,
        primary_key = True
    )

    public_key: Mapped[bytes] = mapped_column(
        LargeBinary,
        unique = True,
        nullable = False
    )

    password_hash: Mapped[str] = mapped_column(
        String,
        nullable = False
    )

    first_name: Mapped[str] = mapped_column(
        String(20),
        CheckConstraint("length(first_name) BETWEEN 1 AND 20"),
        nullable = False
    )

    last_name: Mapped[str] = mapped_column(
        String(20)
        CheckConstraint("length(last_name) BETWEEN 1 AND 20")
        nullable = False
    )