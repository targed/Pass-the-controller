from sqlalchemy import String, LargeBinary, CheckConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class Friends_List(Base):
    __tablename__ = "FRIENDS_LIST"

    owner_user_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        foreign_keys(USER.user_id),
        nullable = False
    )

    friend_user_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        nullable = False
    )

    friend_public_key: Mapped[bytes] = mapped_column(
        LargeBinary,
        nullable = False
    )

    created_at: Mapped[str] = mapped_column(
        String,
        nullable = False
    )

    accepted_at: Mapped[str | None] = mapped_column(
        String,
        CheckConstraint("accepted_at == PENDING OR accepted_at == ACCEPTED OR accepted_at == REJECTED"),
        nullable = True
    )

    status: Mapped[str] = mapped_column(
        String,
        nullable = True
    )