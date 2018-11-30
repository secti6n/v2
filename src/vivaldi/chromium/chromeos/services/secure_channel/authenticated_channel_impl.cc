// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/services/secure_channel/authenticated_channel_impl.h"

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/no_destructor.h"
#include "chromeos/components/proximity_auth/logging/logging.h"

namespace chromeos {

namespace secure_channel {

// static
AuthenticatedChannelImpl::Factory*
    AuthenticatedChannelImpl::Factory::test_factory_ = nullptr;

// static
AuthenticatedChannelImpl::Factory* AuthenticatedChannelImpl::Factory::Get() {
  if (test_factory_)
    return test_factory_;

  static base::NoDestructor<AuthenticatedChannelImpl::Factory> factory;
  return factory.get();
}

// static
void AuthenticatedChannelImpl::Factory::SetFactoryForTesting(
    Factory* test_factory) {
  test_factory_ = test_factory;
}

std::unique_ptr<AuthenticatedChannel>
AuthenticatedChannelImpl::Factory::BuildInstance(
    const std::vector<mojom::ConnectionCreationDetail>&
        connection_creation_details,
    std::unique_ptr<cryptauth::SecureChannel> secure_channel) {
  return base::WrapUnique(new AuthenticatedChannelImpl(
      connection_creation_details, std::move(secure_channel)));
}

AuthenticatedChannelImpl::AuthenticatedChannelImpl(
    const std::vector<mojom::ConnectionCreationDetail>&
        connection_creation_details,
    std::unique_ptr<cryptauth::SecureChannel> secure_channel)
    : AuthenticatedChannel(),
      connection_creation_details_(connection_creation_details),
      secure_channel_(std::move(secure_channel)) {
  // |secure_channel_| should be a valid and already authenticated.
  DCHECK(secure_channel_);
  DCHECK_EQ(secure_channel_->status(),
            cryptauth::SecureChannel::Status::AUTHENTICATED);

  secure_channel_->AddObserver(this);
}

AuthenticatedChannelImpl::~AuthenticatedChannelImpl() {
  secure_channel_->RemoveObserver(this);
}

void AuthenticatedChannelImpl::GetConnectionMetadata(
    base::OnceCallback<void(mojom::ConnectionMetadataPtr)> callback) {
  secure_channel_->GetConnectionRssi(
      base::BindOnce(&AuthenticatedChannelImpl::OnRssiFetched,
                     base::Unretained(this), std::move(callback)));
}

void AuthenticatedChannelImpl::PerformSendMessage(
    const std::string& feature,
    const std::string& payload,
    base::OnceClosure on_sent_callback) {
  DCHECK_EQ(secure_channel_->status(),
            cryptauth::SecureChannel::Status::AUTHENTICATED);

  int sequence_number = secure_channel_->SendMessage(feature, payload);

  if (base::ContainsKey(sequence_number_to_callback_map_, sequence_number)) {
    PA_LOG(ERROR) << "AuthenticatedChannelImpl::SendMessage(): Started sending "
                  << "a message whose sequence number already exists in the "
                  << "map.";
    NOTREACHED();
  }

  sequence_number_to_callback_map_[sequence_number] =
      std::move(on_sent_callback);
}

void AuthenticatedChannelImpl::PerformDisconnection() {
  secure_channel_->Disconnect();
}

void AuthenticatedChannelImpl::OnSecureChannelStatusChanged(
    cryptauth::SecureChannel* secure_channel,
    const cryptauth::SecureChannel::Status& old_status,
    const cryptauth::SecureChannel::Status& new_status) {
  DCHECK_EQ(secure_channel_.get(), secure_channel);

  // The only expected status changes are AUTHENTICATED => DISCONNECTING,
  // AUTHENTICATED => DISCONNECTED, and DISCONNECTING => DISCONNECTED.
  DCHECK(old_status == cryptauth::SecureChannel::Status::AUTHENTICATED ||
         old_status == cryptauth::SecureChannel::Status::DISCONNECTING);
  DCHECK(new_status == cryptauth::SecureChannel::Status::DISCONNECTING ||
         new_status == cryptauth::SecureChannel::Status::DISCONNECTED);

  if (new_status == cryptauth::SecureChannel::Status::DISCONNECTED)
    NotifyDisconnected();
}

void AuthenticatedChannelImpl::OnMessageReceived(
    cryptauth::SecureChannel* secure_channel,
    const std::string& feature,
    const std::string& payload) {
  DCHECK_EQ(secure_channel_.get(), secure_channel);
  NotifyMessageReceived(feature, payload);
}

void AuthenticatedChannelImpl::OnMessageSent(
    cryptauth::SecureChannel* secure_channel,
    int sequence_number) {
  DCHECK_EQ(secure_channel_.get(), secure_channel);

  if (!base::ContainsKey(sequence_number_to_callback_map_, sequence_number)) {
    PA_LOG(WARNING) << "AuthenticatedChannelImpl::OnMessageSent(): Sent a "
                    << "message whose sequence number did not exist in the "
                    << "map. Disregarding.";
    // Note: No DCHECK() is performed here, since |secure_channel_| could have
    // already been in the process of sending a message before the
    // AuthenticatedChannelImpl object was created.
    return;
  }

  std::move(sequence_number_to_callback_map_[sequence_number]).Run();
  sequence_number_to_callback_map_.erase(sequence_number);
}

void AuthenticatedChannelImpl::OnRssiFetched(
    base::OnceCallback<void(mojom::ConnectionMetadataPtr)> callback,
    base::Optional<int32_t> current_rssi) {
  mojom::BluetoothConnectionMetadataPtr bluetooth_connection_metadata_ptr;
  if (current_rssi) {
    bluetooth_connection_metadata_ptr =
        mojom::BluetoothConnectionMetadata::New(*current_rssi);
  }

  // The SecureChannel must have channel binding data if it is authenticated.
  DCHECK(secure_channel_->GetChannelBindingData());

  std::move(callback).Run(mojom::ConnectionMetadata::New(
      connection_creation_details_,
      std::move(bluetooth_connection_metadata_ptr),
      *secure_channel_->GetChannelBindingData()));
}

}  // namespace secure_channel

}  // namespace chromeos