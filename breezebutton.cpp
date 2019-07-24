/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "breezebutton.h"

#include <KDecoration2/DecoratedClient>
#include <KColorUtils>

#include <QPainter>

namespace Breeze
{

    using KDecoration2::ColorRole;
    using KDecoration2::ColorGroup;
    using KDecoration2::DecorationButtonType;


    //__________________________________________________________________
    Button::Button(DecorationButtonType type, Decoration* decoration, QObject* parent)
        : DecorationButton(type, decoration, parent)
        , m_animation( new QPropertyAnimation( this ) )
    {

        // setup animation
        m_animation->setStartValue( 0 );
        m_animation->setEndValue( 1.0 );
        m_animation->setTargetObject( this );
        m_animation->setPropertyName( "opacity" );
        m_animation->setEasingCurve( QEasingCurve::InOutQuad );

        // setup default geometry
        const int height = decoration->buttonHeight();
        const int width = height * (type == DecorationButtonType::Menu ? 1.0 : 1.5);
        setGeometry(QRect(0, 0, width, height));
        setIconSize(QSize( width, height ));

        // connections
        connect(decoration->client().data(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
        connect(decoration->settings().data(), &KDecoration2::DecorationSettings::reconfigured, this, &Button::reconfigure);
        connect( this, &KDecoration2::DecorationButton::hoveredChanged, this, &Button::updateAnimationState );

        reconfigure();

    }

    //__________________________________________________________________
    Button::Button(QObject *parent, const QVariantList &args)
        : Button(args.at(0).value<DecorationButtonType>(), args.at(1).value<Decoration*>(), parent)
    {
        m_flag = FlagStandalone;
        //! icon size must return to !valid because it was altered from the default constructor,
        //! in Standalone mode the button is not using the decoration metrics but its geometry
        m_iconSize = QSize(-1, -1);
    }
            
    //__________________________________________________________________
    Button *Button::create(DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
    {
        if (auto d = qobject_cast<Decoration*>(decoration))
        {
            Button *b = new Button(type, d, parent);
            switch( type )
            {

                case DecorationButtonType::Close:
                b->setVisible( d->client().data()->isCloseable() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::closeableChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::Maximize:
                b->setVisible( d->client().data()->isMaximizeable() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::maximizeableChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::Minimize:
                b->setVisible( d->client().data()->isMinimizeable() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::minimizeableChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::ContextHelp:
                b->setVisible( d->client().data()->providesContextHelp() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::providesContextHelpChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::Shade:
                b->setVisible( d->client().data()->isShadeable() );
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::shadeableChanged, b, &Breeze::Button::setVisible );
                break;

                case DecorationButtonType::Menu:
                QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::iconChanged, b, [b]() { b->update(); });
                break;

                default: break;

            }

            return b;
        }

        return nullptr;

    }

    //__________________________________________________________________
    void Button::paint(QPainter *painter, const QRect &repaintRegion)
    {
        Q_UNUSED(repaintRegion)

        if (!decoration()) return;

        painter->save();

        // translate from offset
        if( m_flag == FlagFirstInList ) painter->translate( m_offset );
        else painter->translate( 0, m_offset.y() );

        if( !m_iconSize.isValid() ) m_iconSize = geometry().size().toSize();

        // menu button
        if (type() == DecorationButtonType::Menu)
        {

            // const QRectF iconRect( geometry().topLeft(), m_iconSize );
            const int menuIconSize = (qobject_cast<Decoration *>(decoration()))->iconSize();
            const qreal topLeft = (geometry().width()/2) - (menuIconSize/2);

            const QRectF iconRect(topLeft + geometry().left(), topLeft + geometry().top(), menuIconSize, menuIconSize);
            decoration()->client().data()->icon().paint(painter, iconRect.toRect());

        } else {

            drawIcon( painter );

        }

        painter->restore();

    }

    //__________________________________________________________________
    void Button::drawIcon( QPainter *painter ) const
    {

        painter->setRenderHints( QPainter::Antialiasing );

        /*
        scale painter so that its window matches QRect( 0, 0, 45, 30 )
        this makes all further rendering and scaling simpler
        all further rendering is preformed inside QRect( 0, 0, 45, 30 )
        */
        painter->translate( geometry().topLeft() );

        const qreal height( m_iconSize.height() );
        const qreal width( m_iconSize.width() );
        if ( height != 30 )
            painter->scale( width/45, height/30 );

        // render background
        const QColor backgroundColor( this->backgroundColor() );

        auto d = qobject_cast<Decoration*>( decoration() );
        bool isInactive(d && !d->client().data()->isActive()
                        && !isHovered() && !isPressed()
                        && m_animation->state() != QPropertyAnimation::Running);
        QColor inactiveCol(Qt::gray);
        if (isInactive)
        {
            int gray = qGray(d->titleBarColor().rgb());
            if (gray <= 200) {
                gray += 55;
                gray = qMax(gray, 115);
            }
            else gray -= 45;
            inactiveCol = QColor(gray, gray, gray);
        }

        // render mark
        const QColor foregroundColor( this->foregroundColor() );
        if( foregroundColor.isValid() )
        {

            // setup painter
            QPen pen( foregroundColor );
            pen.setCapStyle( Qt::FlatCap );
            pen.setJoinStyle( Qt::MiterJoin );
            // pen.setWidthF( 1.0*qMax((qreal)1.0, 30/width ) );
            pen.setWidthF( 1.0 );

            switch( type() )
            {

                case DecorationButtonType::Close:
                {
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawRect( QRectF( 0, 0, 45, 30 ) );
                    }
                    pen.setWidthF( 1.1 );
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawLine( QPointF( 18, 10 ), QPointF( 28, 20 ) );
                    painter->drawLine( QPointF( 18, 20 ), QPointF( 28, 10 ) );
                    pen.setWidthF( 1.0 );
                    break;
                }

                case DecorationButtonType::Maximize:
                {
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawRect( QRectF( 0, 0, 45, 30 ) );
                    }

                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    if (isChecked())
                    {
                        painter->drawRect(QRectF(18.5, 12.5, 7.0, 7.0));
                        painter->drawPolyline(QPolygonF()
                                                << QPointF(20.5, 12.5) << QPointF(20.5, 10.5) << QPointF(27.5, 10.5) << QPointF(27.5, 17.5) << QPointF(25.5, 17.5));
                    }
                    else {
                        painter->drawRect(QRectF(18.5, 10.5, 9.0, 9.0));
                    }

                    break;
                }

                case DecorationButtonType::Minimize:
                {
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawRect( QRectF( 0, 0, 45, 30 ) );
                    }

                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawLine( QPointF( 18, 15.5 ), QPointF( 28, 15.5 ) );

                    break;
                }

                case DecorationButtonType::OnAllDesktops:
                {
                    painter->setPen( Qt::NoPen );
                    if( backgroundColor.isValid() )
                    {
                        painter->setBrush( backgroundColor );
                        painter->drawRect( QRectF( 0, 0, 45, 30 ) );
                    }
                    painter->setBrush( foregroundColor );

                    if( isChecked()) {

                        // outer ring
                        painter->drawRect( QRectF( 16, 9, 12, 12 ) );

                        // center dot
                        QColor backgroundColor( this->backgroundColor() );
                        if( !backgroundColor.isValid() && d ) backgroundColor = d->titleBarColor();

                        if( backgroundColor.isValid() )
                        {
                            painter->setBrush( backgroundColor );
                            painter->drawRect( QRectF( 21, 14, 2, 2 ) );
                        }

                    } else {

                        painter->drawPolygon( QPolygonF()
                            << QPointF( 19.5, 14.5 )
                            << QPointF( 25, 9 )
                            << QPointF( 28, 12 )
                            << QPointF( 22.5, 17.5 ) );

                        painter->setPen( pen );
                        painter->drawLine( QPointF( 18.5, 13.5 ), QPointF( 23.5, 18.5 ) );
                        painter->drawLine( QPointF( 25, 12 ), QPointF( 17.5, 19.5 ) );
                    }
                    break;
                }

                case DecorationButtonType::Shade:
                {
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawRect( QRectF( 0, 0, 45, 30 ) );
                    }
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawLine( 18, 12, 26, 12 );
                    if (isChecked()) {
                        painter->drawPolyline( QPolygonF()
                            << QPointF( 18, 15 )
                            << QPointF( 22, 19 )
                            << QPointF( 26, 15 ) );

                    } else {
                        painter->drawPolyline( QPolygonF()
                            << QPointF( 18, 19 )
                            << QPointF( 22, 15 )
                            << QPointF( 26, 19 ) );
                    }

                    break;

                }

                case DecorationButtonType::KeepBelow:
                {
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawRect( QRectF( 0, 0, 45, 30 ) );
                    }
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawPolyline( QPolygonF()
                        << QPointF( 18, 11 )
                        << QPointF( 22, 15 )
                        << QPointF( 26, 11 ) );

                    painter->drawPolyline( QPolygonF()
                        << QPointF( 18, 15 )
                        << QPointF( 22, 19 )
                        << QPointF( 26, 15 ) );
                    break;

                }

                case DecorationButtonType::KeepAbove:
                {
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawRect( QRectF( 0, 0, 45, 30 ) );
                    }
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawPolyline( QPolygonF()
                        << QPointF( 18, 15 )
                        << QPointF( 22, 11 )
                        << QPointF( 26, 15 ) );

                    painter->drawPolyline( QPolygonF()
                        << QPointF( 18, 19 )
                        << QPointF( 22, 15 )
                        << QPointF( 26, 19 ) );
                    break;
                }


                case DecorationButtonType::ApplicationMenu:
                {
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawRect( QRectF( 0, 0, 45, 30 ) );
                    }
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawLine( QPointF( 15, 11.5 ), QPointF( 30, 11.5 ) );
                    painter->drawLine( QPointF( 15, 15.5 ), QPointF( 30, 15.5 ) );
                    painter->drawLine( QPointF( 15, 19.5 ), QPointF( 30, 19.5 ) );
                    break;
                }

                case DecorationButtonType::ContextHelp:
                {
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawRect( QRectF( 0, 0, 45, 30 ) );
                    }
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    QPainterPath path;
                    path.moveTo( 18, 12 );
                    path.arcTo( QRectF( 18, 9.5, 8, 5 ), 180, -180 );
                    path.cubicTo( QPointF(26.5, 15.5), QPointF( 22, 13.5 ), QPointF( 22, 17.5 ) );
                    painter->drawPath( path );

                    painter->drawPoint( 22, 21 );

                    break;
                }

                default: break;

            }

        }

    }

    //__________________________________________________________________
    QColor Button::foregroundColor() const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        if( !d ) {

            return QColor();

        } else if( isPressed() ) {

            return d->titleBarColor();

        /*} else if( type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton() ) {

            return d->titleBarColor();*/

        } else if( ( type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove ) && isChecked() ) {

            return d->titleBarColor();

        } else if( m_animation->state() == QPropertyAnimation::Running ) {

            return KColorUtils::mix( d->fontColor(), d->titleBarColor(), m_opacity );

        } else if( isHovered() ) {

            return d->titleBarColor();

        } else {

            return d->fontColor();

        }

    }

    //__________________________________________________________________
    QColor Button::backgroundColor() const
    {
        auto d = qobject_cast<Decoration*>( decoration() );
        if( !d ) {

            return QColor();

        }

        auto c = d->client().data();
        if( isPressed() ) {

            if( type() == DecorationButtonType::Close ) return c->color( ColorGroup::Warning, ColorRole::Foreground );
            else return KColorUtils::mix( d->titleBarColor(), d->fontColor(), 0.3 );

        } else if( ( type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove ) && isChecked() ) {

            return d->fontColor();

        } else if( m_animation->state() == QPropertyAnimation::Running ) {

            if( type() == DecorationButtonType::Close )
            {
                /*if( d->internalSettings()->outlineCloseButton() )
                {

                    return KColorUtils::mix( d->fontColor(), c->color( ColorGroup::Warning, ColorRole::Foreground ).lighter(), m_opacity );

                } else {*/

                    QColor color( c->color( ColorGroup::Warning, ColorRole::Foreground ).lighter() );
                    color.setAlpha( color.alpha()*m_opacity );
                    return color;

                //}

            } else {

                QColor color( d->fontColor() );
                color.setAlpha( color.alpha()*m_opacity );
                return color;

            }

        } else if( isHovered() ) {

            if( type() == DecorationButtonType::Close ) return c->color( ColorGroup::Warning, ColorRole::Foreground ).lighter();
            else return d->fontColor();

        /*} else if( type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton() ) {

            return d->fontColor();*/

        } else {

            return QColor();

        }

    }

    //________________________________________________________________
    void Button::reconfigure()
    {

        // animation
        auto d = qobject_cast<Decoration*>(decoration());
        if( d )  m_animation->setDuration( d->internalSettings()->animationsDuration() );

    }

    //__________________________________________________________________
    void Button::updateAnimationState( bool hovered )
    {

        auto d = qobject_cast<Decoration*>(decoration());
        if( !(d && d->internalSettings()->animationsEnabled() ) ) return;

        QAbstractAnimation::Direction dir = hovered ? QPropertyAnimation::Forward : QPropertyAnimation::Backward;
        if( m_animation->state() == QPropertyAnimation::Running && m_animation->direction() != dir )
            m_animation->stop();
        m_animation->setDirection( dir );
        if( m_animation->state() != QPropertyAnimation::Running ) m_animation->start();

    }

} // namespace
