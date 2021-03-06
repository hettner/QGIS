/***************************************************************************
                         qgspointcloudattributemodel.cpp
                         ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudattributemodel.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudindex.h"
#include "qgsapplication.h"

QgsPointCloudAttributeModel::QgsPointCloudAttributeModel( QObject *parent )
  : QAbstractItemModel( parent )
{

}

void QgsPointCloudAttributeModel::setLayer( QgsPointCloudLayer *layer )
{
  if ( layer && layer->dataProvider() && layer->dataProvider()->index() )
  {
    mLayer = layer;
    setAttributes( layer->dataProvider()->index()->attributes() );
  }
  else
    setAttributes( QgsPointCloudAttributeCollection() );
}

QgsPointCloudLayer *QgsPointCloudAttributeModel::layer()
{
  return mLayer;
}

void QgsPointCloudAttributeModel::setAttributes( const QgsPointCloudAttributeCollection &attributes )
{
  beginResetModel();
  mAttributes = attributes;
  endResetModel();
}

void QgsPointCloudAttributeModel::setAllowEmptyAttributeName( bool allowEmpty )
{
  if ( allowEmpty == mAllowEmpty )
    return;

  if ( allowEmpty )
  {
    beginInsertRows( QModelIndex(), 0, 0 );
    mAllowEmpty = true;
    endInsertRows();
  }
  else
  {
    beginRemoveRows( QModelIndex(), 0, 0 );
    mAllowEmpty = false;
    endRemoveRows();
  }
}

QModelIndex QgsPointCloudAttributeModel::indexFromName( const QString &name )
{
  if ( !name.isEmpty() )
  {
    int idx = mAttributes.indexOf( name );
    if ( idx >= 0 )
    {
      if ( mAllowEmpty )
        return index( 1 + idx, 0 );
      else
        return index( idx, 0 );
    }
  }

  if ( mAllowEmpty && name.isEmpty() )
    return index( 0, 0 );

  return QModelIndex();
}

QModelIndex QgsPointCloudAttributeModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}

QModelIndex QgsPointCloudAttributeModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

int QgsPointCloudAttributeModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
  {
    return 0;
  }

  return ( mAllowEmpty ? 1 : 0 ) + mAttributes.count();
}

int QgsPointCloudAttributeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsPointCloudAttributeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const bool isEmpty = mAllowEmpty && index.row() == 0;
  const int fieldOffset = mAllowEmpty ? 1 : 0;

  if ( !isEmpty && ( index.row() - fieldOffset >= mAttributes.count() ) )
    return QVariant();

  switch ( role )
  {
    case AttributeNameRole:
    {
      if ( isEmpty )
      {
        return QVariant();
      }
      return mAttributes.at( index.row() - fieldOffset ).name();
    }

    case AttributeSizeRole:
    {
      if ( isEmpty )
      {
        return QVariant();
      }
      return static_cast< int >( mAttributes.at( index.row() - fieldOffset ).size() );
    }

    case AttributeTypeRole:
    {
      if ( isEmpty )
      {
        return QVariant();
      }
      return static_cast< int >( mAttributes.at( index.row() - fieldOffset ).type() );
    }

    case IsEmptyRole:
    {
      return isEmpty;
    }

    case IsNumericRole:
    {
      if ( isEmpty )
      {
        return QVariant();
      }
      return QgsPointCloudAttribute::isNumeric( mAttributes.at( index.row() - fieldOffset ).type() );
    }

    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
    {
      if ( isEmpty )
      {
        return QVariant();
      }
      else if ( role == Qt::ToolTipRole )
      {
        return attributeToolTip( mAttributes.at( index.row() - fieldOffset ) );
      }
      else
        return mAttributes.at( index.row() - fieldOffset ).name();
    }

    case Qt::DecorationRole:
    {
      if ( !isEmpty )
      {
        return iconForAttributeType( mAttributes.at( index.row() - fieldOffset ).type() );
      }
      return QIcon();
    }

    default:
      return QVariant();
  }
}

QString QgsPointCloudAttributeModel::attributeToolTip( const QgsPointCloudAttribute &attribute )
{
  QString toolTip = QStringLiteral( "<b>%1</b>" ).arg( attribute.name() );

  toolTip += QStringLiteral( "<br><font style='font-family:monospace; white-space: nowrap;'>%3</font>" ).arg( attribute.displayType() );

  return toolTip;
}

QIcon QgsPointCloudAttributeModel::iconForAttributeType( QgsPointCloudAttribute::DataType type )
{
  switch ( type )
  {
    case QgsPointCloudAttribute::Short:
    case QgsPointCloudAttribute::Int32:
    {
      return QgsApplication::getThemeIcon( "/mIconFieldInteger.svg" );
    }
    case QgsPointCloudAttribute::Float:
    case QgsPointCloudAttribute::Double:
    {
      return QgsApplication::getThemeIcon( "/mIconFieldFloat.svg" );
    }
    case QgsPointCloudAttribute::Char:
    {
      return QgsApplication::getThemeIcon( "/mIconFieldText.svg" );
    }

  }
  return QIcon();
}
