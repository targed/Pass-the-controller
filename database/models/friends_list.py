from sqlalchemy import String, LargeBinary, CheckConstraint, ForeignKey
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base


class FriendsList(Base):
    __tablename__ = "FRIENDS_LIST"
   

    owner_user_id: Mapped[str] = mapped_column(
        String,
        ForeignKey("USER.user_id"),
        primary_key = True,
        nullable = False
    )

    friend_public_key: Mapped[bytes] = mapped_column(
        LargeBinary,
        primary_key = True,
        nullable = False
    )

    created_at: Mapped[str] = mapped_column(
        String,
        nullable = False
    )

    status: Mapped[str | None] = mapped_column(
        String,
        CheckConstraint("status IN ('PENDING', 'ACCEPTED', 'REJECTED')"),
        nullable = False
    )

    accepted_at: Mapped[str | None] = mapped_column(
        String,
        nullable = True
    )


    direction: Mapped[str] = mapped_column(
        String,
        CheckConstraint("direction IN ('INCOMING', 'OUTGOING')"),
        nullable = False
    )